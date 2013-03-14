# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = %q{librato-metrics-taps}
  s.version = "0.3.4"

  s.authors = [%q{Librato, Inc.}]
  s.date = %q{2012-05-15}
  s.description = %q{Taps for extracting metrics data and publishing to Librato Metrics}
  s.email = %q{metrics@librato.com}
  s.summary = %q{Librato Metrics Taps}

  s.files = `git ls-files`.split("\n")
  s.test_files = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]

  s.homepage = %q{http://github.com/librato/librato-metrics-taps}
  s.licenses = [%q{MIT}]

  s.add_runtime_dependency(%q<jmx4r>, ["= 0.1.4"])
  s.add_runtime_dependency(%q<rest-client>, ["= 1.6.7"])
  s.add_runtime_dependency(%q<trollop>, ["= 1.16.2"])
  s.add_runtime_dependency(%q<jruby-openssl>, ["= 0.7.4"])

  s.add_development_dependency(%q<rake>, [">= 0"])
  s.add_development_dependency(%q<shoulda>, [">= 0"])
  s.add_development_dependency(%q<rdoc>, ["~> 3.12"])
end

