#!/bin/env ruby

require 'nokogiri'
require 'open-uri'
require 'ruby-progressbar'
require 'set'

def vk_doc(ext)
    uri = "https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/#{ext}.html"
    #puts uri
    doc = Nokogiri::HTML(URI.open(uri))
    return doc
end

def vk_name(macro)
    macro = macro.downcase
    macro = macro.delete_suffix('_extension_name')
    macro = macro.gsub(/_2$/, '2')
    macro = macro.gsub(/_color_space$/, '_colorspace')
    split = macro.index('_', 3)
    macro = macro[0..(split-1)].upcase+macro[split..-1]
    return macro
end

def vk_macro(name)
    name = name.upcase
    name = name.gsub(/2$/, '_2')
    name = name.gsub(/MAINTENANCE_2$/, 'MAINTENANCE2')
    name = name.gsub(/_COLORSPACE$/, '_COLOR_SPACE')
    name = name + '_EXTENSION_NAME'
    return name
end

def vk_requires(extname)
    exts = []
    doc = vk_doc(extname)
    doc.css('h2#_extension_and_version_dependencies').each do |h2|
        ul = h2.parent.css('div.sectionbody ul').first
        ul.css('li a').each do |a|
            exts << a.text
            yield exts.last if block_given?
        end
    end
    return exts
end

# vk_requires('VK_KHR_16bit_storage') do |name|
#     puts name
#     macro = vk_macro(name)
#     name2 = vk_name(macro)
#     puts "#{name} : #{macro} -> #{name2}"
#     raise name2 if name != name2
# end

EXPORTS = 'C:\Code\ppe\Source\External\vulkan\Public\vulkan-exports.inl'
INSTANCEEXT_RE = /^VK_INSTANCE_LEVEL_EXTENSION\(\s*(\w+)/
DEVICEEXT_RE = /^VK_DEVICE_LEVEL_EXTENSION\(\s*(\w+)/

exports = File.readlines(EXPORTS)

$visited = Set.new

def rewrite_macro(type, macro, input)
    $visited << macro
    name = vk_name(macro)
    requires = vk_requires(name).collect{|x| vk_macro(x)}
    requires.each do |dep|
        raise "#{macro} -> #{dep}" unless $visited.include?(dep)
    end
    outp = requires.empty? ?
        "#{type}( #{macro} )\n" :
        "#{type}( #{macro}, #{requires.join(', ')} )\n"
    if outp != input
        $pbar.log input.inspect
        $pbar.log outp.inspect
        input.replace(outp)
        return true
    end
    return false
end

rewrite = false
$pbar = ProgressBar.create(title: EXPORTS, total:exports.length, progress_mark:'█'.encode('utf-8'))
exports.each_with_index do |input, line|
    if m = input.match(DEVICEEXT_RE)
        rewrite |= rewrite_macro('VK_DEVICE_LEVEL_EXTENSION', m[1], input)
    elsif m = input.match(INSTANCEEXT_RE)
        rewrite |= rewrite_macro('VK_INSTANCE_LEVEL_EXTENSION', m[1], input)
    end
    $pbar.increment
end

if rewrite
    dst = EXPORTS+'.txt'
    puts "rewrite to #{dst}"
    File.open(dst, 'w') do |fd|
        fd.puts exports
    end
end
