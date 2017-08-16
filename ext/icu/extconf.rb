require 'mkmf'

require 'rubygems'

ROOT = File.expand_path(File.join(File.dirname(__FILE__), '..', '..'))

# Utility functions

def asplode(lib)
  abort "-----\n#{lib} is missing.  Please locate mkmf.log to investigate how it is failing.\n-----"
end

def using_system_libraries?
  arg_config('--use-system-libraries', !!ENV['ICU_USE_SYSTEM_LIBRARIES'])
end

def lib_a(ldflag)
  case ldflag
  when /\A-l(.+)/
    "lib#{$1}.#{$LIBEXT}"
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

  have_library 'icuuc' or abort 'libicuuc missing'
  have_library 'icudata' or abort 'libicudata missing'
else
  message "Building ICU from source. TODO: Loading in OS X. Use brew icu4c instead.\n"

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
        hash[key] = ENV[key].dup.to_s rescue ''
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
            "--enable-static",
            "--enable-shared",
            "--disable-renaming"
        ]
        env['CFLAGS'] = "-fPIC #{env['CFLAGS']}"
        env['CPPFLAGS'] = "-DU_CHARSET_IS_UTF8=1 -DU_USING_ICU_NAMESPACE=0 -DU_STATIC_IMPLEMENTATION #{env['CPPFLAGS']}"
        env['CXXFLAGS'] = "-fPIC -fno-exceptions #{env['CXXFLAGS']}"
        env['LDFLAGS'] = "-static-libstdc++ #{env['CFLAGS']}"
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

      @env = env
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

    def configure
      # run as recommend, basically set up compiler and flags
      platform = if RUBY_PLATFORM =~ /mingw|mswin/
                   'MSYS/MSVC'
                 elsif RUBY_PLATFORM =~ /darwin/
                   'MacOSX'
                 else
                   'Linux'
                 end  # double quotes are significant.
      execute('ICU Configure', [@env] + ['./runConfigureICU', platform] + computed_options)
      super
    end

    def work_path
      File.join(Dir.glob("#{tmp_path}/*").find { |d| File.directory?(d) }, 'source')
    end

  end

  message "Using mini_portile version #{MiniPortile::VERSION}\n"

  static_p = enable_config('static', true) or
      message "Static linking is disabled.\n"

  libicu_recipe = ICURecipe.new("libicu", "59.1", static_p) do |recipe|
    recipe.files = [{
                        url: "https://downloads.sourceforge.net/project/icu/ICU4C/59.1/icu4c-59_1-src.tgz?r=&ts=1501595646",
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
  end

  libicu_recipe.tap do |recipe|
    checkpoint = "#{recipe.target}/#{recipe.name}-#{recipe.version}-#{recipe.host}.installed"
    unless File.exist?(checkpoint)
      recipe.cook
      FileUtils.touch checkpoint
    end

    recipe.activate
  end

  libname = libicu_recipe.name[/\Alib(.+)\z/, 1]
  # TODO: build with pkg-config
  # Should do like PKG_CONFIG_PATH=/root/icu4r/ports/x86_64-pc-linux-gnu/libicu/59.1/lib/pkgconfig/ pkg-config --static  icu-uc
  File.join(libicu_recipe.path, "bin", "#{libname}-config").tap do |config|
    # call config scripts explicit with 'sh' for compat with Windows
    `sh #{config} --ldflags`.strip.shellsplit.each do |arg|
      case arg
      when /\A-L(.+)\z/
        # Prioritize ports' directories
        if $1.start_with?(ROOT + '/')
          $LIBPATH.unshift($1)
        else
          $LIBPATH.push($1)
        end
      when /\A-l./
        $LDFLAGS << ' ' << arg
      else
        $LDFLAGS << ' ' << arg.shellescape
      end
    end
    $INCFLAGS = `sh #{config} --cppflags-searchpath `.strip << ' ' << $INCFLAGS
    $CPPFLAGS = '-DU_DISABLE_RENAMING=1 -DU_CHARSET_IS_UTF8=1 -DU_USING_ICU_NAMESPACE=0 -DU_STATIC_IMPLEMENTATION' << ' ' << $CPPFLAGS
    $CFLAGS = `sh #{config} --cflags`.strip << $CFLAGS
  end

  $LIBPATH = ["#{libicu_recipe.path}/lib"] | $LIBPATH if libicu_recipe
  $libs = ["-licui18n", "-licuuc", "-licudata"].map do |arg|
   File.join(libicu_recipe.path, 'lib', lib_a(arg))
  end.shelljoin
end

$CFLAGS << ' -O3 -funroll-loops -std=c99'
$CFLAGS << ' -Wextra -O0 -ggdb3' if ENV['DEBUG']

have_func('u_init', 'unicode/uclean.h') ||
  have_library('icui18n', 'u_init', 'unicode/uclean.h') ||
  have_library('libicui18n', 'u_init', 'unicode/uclean.h') or
  asplode('libicui18n')
have_func('u_errorName')

create_makefile('icu/icu')
