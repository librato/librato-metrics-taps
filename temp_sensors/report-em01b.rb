#!/usr/bin/env ruby

require 'cgi'
require 'optparse'

require 'rubygems'
require 'rest-client'

SOURCES=['em01b-r2', 'em01b-r4']

$options = {
  :uri => 'metrics-stg.heroku.com/v1/metrics.json',
  :user => nil,
  :pass => nil,
  :path => nil
}

optparse = OptionParser.new do|opts|
  opts.banner = "Usage: report-em01b [options]\n"

  # help
  opts.on( '-h', '--help', 'Display this screen' ) do
    puts opts
    exit
  end

  # API
  opts.on('--uri <API URI>', 'Set API address') do |uri|
    $options[:uri] = uri
  end

  # email
  opts.on('--email <EMAIL>', 'Set email') do |email|
    $options[:user] = CGI.escape(email)
  end

  # token
  opts.on('--token <TOKEN>', 'Set token') do |token|
    $options[:pass] = token
  end

  # path to check utility
  opts.on('--path <PATH>', 'Path to check utility') do |path|
    $options[:path] = path
  end
end

optparse.parse!(ARGV)

unless $options[:user] && $options[:pass]
  puts "Must specify --email and --token"
  exit 1
end

unless $options[:path] && File.exist?($options[:path])
  puts "Can't find executable at: #{$options[:path]}"
  exit 1
end

gauges = {}
SOURCES.each_with_index do |src, i|

  str = %x{#{$options[:path]} #{src} 2>&1}.chomp
  unless $?.exited? && $?.exitstatus == 0
    puts "Failed to execute: #{$options[:path]}"
    exit 1
  end

  m = str.match(/.*TF: ([\.0-9]+)HU:[ ]{0,}([\.0-9]+)%IL:[ ]{0,}([\.0-9]+)[ ]{0,}/)

  gauges["#{i}_0"] = {
    :name => 'sfdc::temperature',
    :source => src,
    :value => Float(m[1])
  }

  gauges["#{i}_1"] = {
    :name => 'sfdc::humidity',
    :source => src,
    :value => Float(m[2])
  }

  gauges["#{i}_2"] = {
    :name => 'sfdc::illumination',
    :source => src,
    :value => Float(m[3])
  }
end

RestClient.post "https://#{$options[:user]}:#{$options[:pass]}@#{$options[:uri]}", :gauges => gauges
