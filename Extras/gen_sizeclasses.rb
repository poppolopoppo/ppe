#!/bin/env ruby

require 'pp'

POW_N = 2
MIN_CLASS = 19

def align(s) return (s+15) & (~15); end
def size_class(s)
    s = align(s)
    i = Math.log2((s - 1) | 1).floor.to_i
    return ((i << POW_N) + ((s - 1) >> (i - POW_N)) - MIN_CLASS)
end

SIZE_CLASSES = []
s = 16
while s <= 32 * 1024
    c = size_class(s)
    SIZE_CLASSES[c] = s
    s += 16
end

COLUMNS = 4
i = 0
while i < SIZE_CLASSES.length
    r = SIZE_CLASSES[(i)...(i+COLUMNS)]
    puts "\t/* #{i.to_s.rjust(2)} */    #{r.collect!{ |x| (x.nil? ? 0 : x).to_s.ljust(6) }.join(', ')}#{i + COLUMNS < SIZE_CLASSES.length ? ',' : ''}"
    i += COLUMNS
end
