require 'mkmf'

require 'rubygems'

ROOT = File.expand_path(File.join(File.dirname(__FILE__), '..', '..'))

# Utility functions

def using_system_libraries?
  arg_config('--use-system-libraries', !!ENV['ICU_USE_SYSTEM_LIBRARIES'])
end

# From Nokogiri:
# When using rake-compiler-dock on Windows, the underlying Virtualbox shared
# folders don't support symlinks, but libiconv expects it for a build on
# Linux. We work around this limitation by using the temp dir for cooking.
def chdir_for_build
  build_dir = ENV['RCD_HOST_RUBY_PLATFORM'].to_s =~ /mingw|mswin|cygwin/ ? '/tmp' : '.'
  Dir.chdir(build_dir) do
    yield
  end
end

# Building with system ICU

if using_system_libraries?
  message "Building ICU using system libraries.\n"

  unless dir_config('icu').any?
    base = if !`which brew`.empty?
             `brew --prefix`.strip
           elsif File.exists?("/usr/local/Cellar/icu4c")
             '/usr/local/Cellar'
           end

    if base and icu4c = Dir[File.join(base, 'Cellar/icu4c/*')].sort.last
      $INCFLAGS << " -I#{icu4c}/include "
      $LDFLAGS  << " -L#{icu4c}/lib "
    end
  end

  unless have_library 'icui18n' and have_header 'unicode/ucnv.h'
    STDERR.puts <<-EOS
************************************************************************
icu not found.
install by brew install icu4c or apt-get install libicu-dev)
************************************************************************ww
    EOS

    exit(1)
  end
else
  message "Building ICU from source.\n"

  # The gem version constraint in the Rakefile is not respected at install time.
  # Keep this version in sync with the one in the Rakefile !
  require 'rubygems'
  gem 'mini_portile2', '~> 2.2.0'
  require 'mini_portile2'

  # Checkout the source code of ICU.
  # http://site.icu-project.org/download/
  # http://userguide.icu-project.org/howtouseicu
  # Also check the readme of ICU release file.
  class ICURecipe < MiniPortile
    def initialize(name, version, static_p)
      super(name, version)
      self.target = File.join(ROOT, "ports")
      # Prefer host_alias over host in order to use i586-mingw32msvc as
      # correct compiler prefix for cross build, but use host if not set.
      self.host = RbConfig::CONFIG["host_alias"].empty? ? RbConfig::CONFIG["host"] : RbConfig::CONFIG["host_alias"]
      self.patch_files = Dir[File.join(ROOT, "patches", name, "*.patch")].sort
      self.configure_options << "--libdir=#{File.join(self.path, "lib")}"

      yield self

      env = Hash.new do |hash, key|
        hash[key] = "#{ENV[key]}"  # (ENV[key].dup rescue '')
      end

      self.configure_options.flatten!

      self.configure_options.delete_if do |option|
        case option
          when /\A(\w+)=(.*)\z/
            env[$1] = $2
            true
          else
            false
        end
      end

      if static_p
        self.configure_options += [
            "--disable-shared",
            "--enable-static",
        ]
        env['CFLAGS'] = "-fPIC #{env['CFLAGS']}"
      else
        self.configure_options += [
            "--enable-shared",
            "--disable-static",
        ]
      end

      if RbConfig::CONFIG['target_cpu'] == 'universal'
        %w[CFLAGS LDFLAGS].each do |key|
          unless env[key].include?('-arch')
            env[key] += ' ' + RbConfig::CONFIG['ARCH_FLAG']
          end
        end
      end

      self.configure_options += env.map do |key, value|
        "#{key}=#{value}"
      end
    end

    def cook

      message <<-"EOS"
************************************************************************
IMPORTANT NOTICE:

Building ICU with a packaged version of #{name}-#{version}#{'.' if self.patch_files.empty?}
      EOS

      unless self.patch_files.empty?
        message "with the following patches applied:\n"

        self.patch_files.each do |patch|
          message "\t- %s\n" % File.basename(patch)
        end
      end

      message <<-"EOS"

    gem install icu -- --use-system-libraries

If you are using Bundler, tell it to use the option:

    bundle config build.icu --use-system-libraries
    bundle install
      EOS

      message <<-"EOS"
************************************************************************
      EOS
      super
    end

    def work_path
      File.join(Dir.glob("#{tmp_path}/*").find { |d| File.directory?(d) }, 'source')
    end

  end

  message "Using mini_portile version #{MiniPortile::VERSION}\n"

  static_p = enable_config('static', true) or
      message "Static linking is disabled.\n"
  recipes = []
  
  libicu_recipe = ICURecipe.new("libicu", "59.1", static_p) do |recipe|
    recipe.files = [{
                        url: "http://download.icu-project.org/files/icu4c/59.1/icu4c-59_1-src.tgz",
                        sha256: "7132fdaf9379429d004005217f10e00b7d2319d0fea22bdfddef8991c45b75fe"
                        # gpg: Signature made Fri Apr 14 21:00:23 2017 CEST using RSA key ID 4FB419E3
                        # gpg: requesting key 4FB419E3 from hkps server hkps.pool.sks-keyservers.net
                        # gpg: key 4FB419E3: public key "Steven R. Loomis (filfla-signing) <srloomis@us.ibm.com>" imported
                        # gpg: 3 marginal(s) needed, 1 complete(s) needed, PGP trust model
                        # gpg: depth: 0  valid:   2  signed:   1  trust: 0-, 0q, 0n, 0m, 0f, 2u
                        # gpg: depth: 1  valid:   1  signed:   0  trust: 1-, 0q, 0n, 0m, 0f, 0u
                        # gpg: next trustdb check due at 2018-08-19
                        # gpg: Total number processed: 1
                        # gpg:               imported: 1  (RSA: 1)
                        # gpg: Good signature from "Steven R. Loomis (filfla-signing) <srloomis@us.ibm.com>" [unknown]
                        # gpg:                 aka "Steven R. Loomis (filfla-signing) <srl295@gmail.com>" [unknown]
                        # gpg:                 aka "Steven R. Loomis (filfla-signing) <srl@icu-project.org>" [unknown]
                        # gpg:                 aka "[jpeg image of size 4680]" [unknown]
                        # gpg: WARNING: This key is not certified with a trusted signature!
                        # gpg:          There is no indication that the signature belongs to the owner.
                        # Primary key fingerprint: BA90 283A 60D6 7BA0 DD91  0A89 3932 080F 4FB4 19E3
                    }]
    recipe.configure_options += [
        "CPPFLAGS=-Wall",
        "CFLAGS=-O2 -g",
        "CXXFLAGS=-O2 -g",
        "LDFLAGS="
    ]
  end
  recipes.push libicu_recipe

  recipes.each do |recipe|
    checkpoint = "#{recipe.target}/#{recipe.name}-#{recipe.version}-#{recipe.host}.installed"
    unless File.exist?(checkpoint)
      recipe.cook
      FileUtils.touch checkpoint
    end

    recipe.activate
  end

  $libs = $libs.shellsplit.tap do |libs|
    recipes.each do |recipe|
      libname = recipe.name[/\Alib(.+)\z/, 1]
      File.join(recipe.path, "bin", "#{libname}-config").tap do |config|
        # call config scripts explicit with 'sh' for compat with Windows
        $CPPFLAGS = `sh #{config} --cflags`.strip << ' ' << $CPPFLAGS
        `sh #{config} --ldflags`.strip.shellsplit.each do |arg|
          case arg
          when /\A-L(.+)\z/
            # Prioritize ports' directories
            if $1.start_with?(ROOT + '/')
              $LIBPATH = [$1] | $LIBPATH
            else
              $LIBPATH = $LIBPATH | [$1]
            end
          when /\A-l./
            libs.unshift(arg)
          else
            $LDFLAGS << ' ' << arg.shellescape
          end
        end
        $INCFLAGS = `sh #{config} --cppflags-searchpath `.strip << ' ' << $INCFLAGS
      end
    end
  end.shelljoin

end

have_library 'icuuc' or abort 'libicuuc missing'
have_library 'icudata' or abort 'libicudata missing'

$CFLAGS << ' -Wall -funroll-loops'
$CFLAGS << ' -Wextra -O0 -ggdb3' if ENV['DEBUG']

create_makefile('icu/icu')
