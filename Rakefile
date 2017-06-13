require 'rubygems'
require 'rake'
require 'rake/clean'
require "rake/extensiontask"
require 'rake/clean'

# Build directory constants
BASEDIR = Pathname(__FILE__).dirname
SPECDIR = BASEDIR + 'spec'
LIBDIR  = BASEDIR + 'lib'
EXTDIR  = BASEDIR + 'ext'
PKGDIR  = BASEDIR + 'pkg'
TMPDIR  = BASEDIR + 'tmp'

CLEAN.include(PKGDIR.to_s, TMPDIR.to_s)

require 'bundler'
Bundler::GemHelper.install_tasks

require 'rspec/core/rake_task'
RSpec::Core::RakeTask.new(:spec) do |spec|
  spec.pattern = 'spec/**/*_spec.rb'
  spec.rspec_opts = '--backtrace'
end

RSpec::Core::RakeTask.new(:rcov) do |spec|
  spec.pattern = 'spec/**/*_spec.rb'
  spec.rcov = true
end

gem = Gem::Specification.load(File.dirname(__FILE__) + '/icu.gemspec')
Rake::ExtensionTask.new('icu', gem) do |ext|
  ext.lib_dir = File.join('lib', 'icu')
end

Rake::Task[:spec].prerequisites << :compile

task :default => :spec
