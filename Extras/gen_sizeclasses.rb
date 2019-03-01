#!/bin/env ruby

require 'pp'
require 'prime'

MaxLoadFactor = 80
SlackFactor = (((100 - MaxLoadFactor) * 128) / 100)
def make_slack(n) (n * SlackFactor) >> 7 end
def bounded(n, p) (n * p) & 0xFFFFFFFF end


PRIME_NUMBERS = true

SIZE_CLASSES = []
PRIME_CLASSES = []

COLUMNS = 4
MAX_VALUE = 65536+make_slack(65536)

POW_N = 2
MIN_CLASS = PRIME_NUMBERS ? 3 : 19


class HashTable
    attr_reader :data, :size
    attr_reader :max_distance
    def initialize(size)
        @size = size
        @data = Array.new(@size)
        @max_distance = 0
    end
    def add(x)
        h = x % @size
        d = 0
        while @data[h]
            return if @data[h] == x
            d += 1
            h = (h + 1) % @size
        end
        @data[h] = x
        @max_distance = d if d > @max_distance
    end
end

if
# https://planetmath.org/goodhashtableprimes

    def size_class(s)
        return Math.log2((s - 1) | 1).ceil.to_i - MIN_CLASS
    end

    4.upto(16) do |i|
        s = 1<<i
        n = 1<<(i+1)

        mi = s
        pi = nil
        ma = 0
        pa = nil
        s.upto(n-1) do |x|
            next unless Prime.prime?(x)
            d = ((s - x).abs - (n - x).abs).abs
            #puts "#{x} (#{d})"
            if d < mi
                pi = x
                mi = d
            end
            if d >= ma
                pa = x
                ma = d
            end
        end

=begin
        r = 10000

        di = 0
        srand(42)
        r.times do
            h = HashTable.new(pi)
            pi.times { |x| h.add(rand(x)) }
            di = h.max_distance if di < h.max_distance
        end

        da = 0
        srand(42)
        r.times do
            h = HashTable.new(pa)
            pa.times { |x| h.add(rand(x)) }
            da = h.max_distance if da < h.max_distance
        end

        puts("*#{'='*40}*")
        puts "#{pi.to_s.rjust(8)} #{pi.to_s(2).rjust(16, '0')} (#{di})"
        puts "#{pa.to_s.rjust(8)} #{pa.to_s(2).rjust(16, '0')} (#{da})"
=end

        p = pi
        s = p - ((p * SlackFactor) >> 7)
        s = 1<<i

        puts "#{p.to_s(2).rjust(16,'0')}"
        puts "#{p} -> #{s} = #{(p - s)*100.0/p}%"

        SIZE_CLASSES[i-MIN_CLASS] = s
        PRIME_CLASSES[i-MIN_CLASS] = p
    end

    s = 16
    while s <= 64 * 1024
        c = size_class(s)
        z = 1<<(MIN_CLASS + c)
        puts "#{s} <= #{SIZE_CLASSES[c]} <= #{PRIME_CLASSES[c]} (#{z})"
        raise "invalid size class" if s > SIZE_CLASSES[c]
        raise "invalid size class" if SIZE_CLASSES[c] != z
        s += 16
    end

    i=0; while i < PRIME_CLASSES.length
        r = PRIME_CLASSES[(i)...(i+COLUMNS)]
        puts "\t/* #{i.to_s.rjust(3)} */    #{r.collect!{ |x| (x.nil? ? 0 : x).to_s.ljust(6) }.join(', ')}#{i + COLUMNS < SIZE_CLASSES.length ? ',' : ''}"
        i += COLUMNS
    end

else

    def align(s) return (s+15) & (~15); end
    def size_class(s)
        s = align(s)
        i = Math.log2((s - 1) | 1).floor.to_i
        return ((i << POW_N) + ((s - 1) >> (i - POW_N)) - MIN_CLASS)
    end

    s = 16
    while s <= 32 * 1024
        c = size_class(s)
        SIZE_CLASSES[c] = s
        s += 16
    end
end

i=0; while i < SIZE_CLASSES.length
    r = SIZE_CLASSES[(i)...(i+COLUMNS)]
    puts "\t/* #{i.to_s.rjust(3)} */    #{r.collect!{ |x| (x.nil? ? 0 : x).to_s.ljust(6) }.join(', ')}#{i + COLUMNS < SIZE_CLASSES.length ? ',' : ''}"
    i += COLUMNS
end

