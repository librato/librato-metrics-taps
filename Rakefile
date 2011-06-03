require 'rubygems'
require 'bundler'
begin
  Bundler.setup(:default, :development)
rescue Bundler::BundlerError => e
  $stderr.puts e.message
  $stderr.puts "Run `bundle install` to install missing gems"
  exit e.status_code
end
require 'rake'

require 'jeweler'
Jeweler::Tasks.new do |gem|
  # gem is a Gem::Specification... see http://docs.rubygems.org/read/chapter/20 for more options
  gem.name = "librato-metrics-taps"
  gem.homepage = "http://github.com/librato/librato-metrics-taps"
  gem.license = "MIT"
  gem.summary = %Q{Librato Metrics Taps}
  gem.description = %Q{Taps for extracting metrics data and publishing to Librato Metrics}
  gem.email = "silverline@librato.com"
  gem.authors = ["Librato, Inc."]

  gem.add_runtime_dependency 'jmx4r', '0.1.3'
  gem.add_runtime_dependency 'rest-client', '1.6.1'
end
Jeweler::RubygemsDotOrgTasks.new

#
# XXX: Rake does not provide a way to remove a task
#
Rake::TaskManager.class_eval do
  def remove_task(task_name)
    @tasks.delete(task_name.to_s)
  end
end

def remove_task(task_name)
  Rake.application.remove_task(task_name)
end

# We don't want to release to rubygems
remove_task :release
desc "Build gemspec, commit, and then git/tag push."
task :release => ['gemspec:release', 'git:release' ]

require 'rake/testtask'
Rake::TestTask.new(:test) do |test|
  test.libs << 'lib' << 'test'
  test.pattern = 'test/**/test_*.rb'
  test.verbose = true
end

require 'rcov/rcovtask'
Rcov::RcovTask.new do |test|
  test.libs << 'test'
  test.pattern = 'test/**/test_*.rb'
  test.verbose = true
end

task :default => :test

require 'rake/rdoctask'
Rake::RDocTask.new do |rdoc|
  version = File.exist?('VERSION') ? File.read('VERSION') : ""

  rdoc.rdoc_dir = 'rdoc'
  rdoc.title = "librato-metrics-taps #{version}"
  rdoc.rdoc_files.include('README*')
  rdoc.rdoc_files.include('lib/**/*.rb')
end
