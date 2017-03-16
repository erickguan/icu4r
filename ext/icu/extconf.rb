require 'mkmf'

require 'rubygems'
gem 'pkg-config', '~> 1.1.7'
require 'pkg-config'

# Utility funtcions

def package_config(pkg, options={})
  package = pkg_config(pkg)
  return package if package

  return nil unless PKGConfig.have_package(pkg)

  cflags  = PKGConfig.cflags(pkg)
  ldflags = PKGConfig.libs_only_L(pkg)
  libs    = PKGConfig.libs_only_l(pkg)

  Logging::message "PKGConfig package configuration for %s\n", pkg
  Logging::message "cflags: %s\nldflags: %s\nlibs: %s\n\n", cflags, ldflags, libs

  [cflags, ldflags, libs]
end

def check_icu_version(version=nil)
end

def using_system_libraries?
  !arg_config('--use-packaged-libraries', !!ENV['ICU_USE_PACKAGED_LIBRARIES'])
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
  # TODO: support building from source
  message "TODO: Building ICU using packaged libraries.\n"
end

have_library 'icuuc' or abort 'libicuuc missing'
have_library 'icudata' or abort 'libicudata missing'

$CFLAGS << ' -Wall -funroll-loops'
$CFLAGS << ' -Wextra -O0 -ggdb3' if ENV['DEBUG']

create_makefile('icu4r-next/icu')
