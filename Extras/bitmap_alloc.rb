#!/bin/env ruby

require 'pp'

MIP_MASKS = [
    0x0000000000000001,
    0x0000000000000006,
    0x0000000000000078,
    0x0000000000007F80,
    0x000000007FFF8000,
    0x7FFFFFFF80000000,
]

MIP_SIZES = [
    2048    * 1024,
    1024    * 1024,
    512     * 1024,
    256     * 1024,
    128     * 1024,
    64      * 1024,
]

BIT_SETMASKS = [
    0x8000000000000000, 0xFFFF80007F807864, 0x80007FFF807F879A, 0xFFFFFF807FF87E74,
    0xFFFF807FFF87F9EC, 0xFF807FFFF87FE7DA, 0x807FFFFF87FF9FBA, 0xFFFFFFF87FFE7F74,
    0xFFFFFF87FFF9FEF4, 0xFFFFF87FFFE7FDEC, 0xFFFF87FFFF9FFBEC, 0xFFF87FFFFE7FF7DA,
    0xFF87FFFFF9FFEFDA, 0xF87FFFFFE7FFDFBA, 0x87FFFFFF9FFFBFBA, 0xFFFFFFFE7FFF7F74,
    0xFFFFFFF9FFFEFF74, 0xFFFFFFE7FFFDFEF4, 0xFFFFFF9FFFFBFEF4, 0xFFFFFE7FFFF7FDEC,
    0xFFFFF9FFFFEFFDEC, 0xFFFFE7FFFFDFFBEC, 0xFFFF9FFFFFBFFBEC, 0xFFFE7FFFFF7FF7DA,
    0xFFF9FFFFFEFFF7DA, 0xFFE7FFFFFDFFEFDA, 0xFF9FFFFFFBFFEFDA, 0xFE7FFFFFF7FFDFBA,
    0xF9FFFFFFEFFFDFBA, 0xE7FFFFFFDFFFBFBA, 0x9FFFFFFFBFFFBFBA, 0xFFFFFFFF7FFF7F74,
    0xFFFFFFFEFFFF7F74, 0xFFFFFFFDFFFEFF74, 0xFFFFFFFBFFFEFF74, 0xFFFFFFF7FFFDFEF4,
    0xFFFFFFEFFFFDFEF4, 0xFFFFFFDFFFFBFEF4, 0xFFFFFFBFFFFBFEF4, 0xFFFFFF7FFFF7FDEC,
    0xFFFFFEFFFFF7FDEC, 0xFFFFFDFFFFEFFDEC, 0xFFFFFBFFFFEFFDEC, 0xFFFFF7FFFFDFFBEC,
    0xFFFFEFFFFFDFFBEC, 0xFFFFDFFFFFBFFBEC, 0xFFFFBFFFFFBFFBEC, 0xFFFF7FFFFF7FF7DA,
    0xFFFEFFFFFF7FF7DA, 0xFFFDFFFFFEFFF7DA, 0xFFFBFFFFFEFFF7DA, 0xFFF7FFFFFDFFEFDA,
    0xFFEFFFFFFDFFEFDA, 0xFFDFFFFFFBFFEFDA, 0xFFBFFFFFFBFFEFDA, 0xFF7FFFFFF7FFDFBA,
    0xFEFFFFFFF7FFDFBA, 0xFDFFFFFFEFFFDFBA, 0xFBFFFFFFEFFFDFBA, 0xF7FFFFFFDFFFBFBA,
    0xEFFFFFFFDFFFBFBA, 0xDFFFFFFFBFFFBFBA, 0xBFFFFFFFBFFFBFBA, 0x0000000000000000
]

BIT_UNSETMASKS = [
    0x7FFFFFFFFFFFFFFF, 0x00007FFF807F879A, 0x7FFF80007F807864, 0x0000007F80078188,
    0x00007F8000780610, 0x007F800007801820, 0x7F80000078006040, 0x0000000780018080,
    0x0000007800060100, 0x0000078000180200, 0x0000780000600400, 0x0007800001800800,
    0x0078000006001000, 0x0780000018002000, 0x7800000060004000, 0x0000000180008000,
    0x0000000600010000, 0x0000001800020000, 0x0000006000040000, 0x0000018000080000,
    0x0000060000100000, 0x0000180000200000, 0x0000600000400000, 0x0001800000800000,
    0x0006000001000000, 0x0018000002000000, 0x0060000004000000, 0x0180000008000000,
    0x0600000010000000, 0x1800000020000000, 0x6000000040000000, 0x0000000080000000,
    0x0000000100000000, 0x0000000200000000, 0x0000000400000000, 0x0000000800000000,
    0x0000001000000000, 0x0000002000000000, 0x0000004000000000, 0x0000008000000000,
    0x0000010000000000, 0x0000020000000000, 0x0000040000000000, 0x0000080000000000,
    0x0000100000000000, 0x0000200000000000, 0x0000400000000000, 0x0000800000000000,
    0x0001000000000000, 0x0002000000000000, 0x0004000000000000, 0x0008000000000000,
    0x0010000000000000, 0x0020000000000000, 0x0040000000000000, 0x0080000000000000,
    0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000,
    0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x0000000000000000,
]

BIT_SIZEMASKS = [
    0x000000008000808B, 0x0000000100000000, 0x0000000200010000, 0x0000000400000000,
    0x0000000800020100, 0x0000001000000000, 0x0000002000040000, 0x0000004000000000,
    0x0000008000080210, 0x0000010000000000, 0x0000020000100000, 0x0000040000000000,
    0x0000080000200400, 0x0000100000000000, 0x0000200000400000, 0x0000400000000000,
    0x0000800000800824, 0x0001000000000000, 0x0002000001000000, 0x0004000000000000,
    0x0008000002001000, 0x0010000000000000, 0x0020000004000000, 0x0040000000000000,
    0x0080000008002040, 0x0100000000000000, 0x0200000010000000, 0x0400000000000000,
    0x0800000020004000, 0x1000000000000000, 0x2000000040000000, 0x4000000000000000,
]

def floor_log2(i) Math.log2(i).floor.to_i end
def miplevel(size) floor_log2((2048 * 1024) / size) end
def mipoffset(level) (1<<level) - 1 end #(2**level) - 1 end

def firstbit(m)
    0.upto(63) do |i|
        return i if (m & (1 << i)) != 0
    end
    raise "unreachable"
end

def popcnt(v)
    n = 0
    0.upto(63) do |i|
        n += 1 if (v & (1<<i)) != 0
    end
    return n
end


class Bitmap
    EMPTY = 0xFFFFFFFFFFFFFFFF
    attr_reader :mask, :sizes
    def initialize()
        @mask = EMPTY
        @sizes = Array.new(32)
    end
    def reset(value = EMPTY) @mask = value end
    def each(index, depth = 0, &block)
        if index < 63
            block.call(index, depth)
            each(child0(index), depth + 1, &block)
            each(child1(index), depth + 1, &block)
        end
    end
    def parent(index) (index - 1) / 2 end
    def child0(index) (2 * index) + 1 end
    def child1(index) (2 * index) + 2 end
    def set?(index)
        return (@mask & (1<<index)) == 0
    end
    def set_index(index, root = index)
        if index < 63 && !set?(index)
            @mask &= ~(1<<index)
            #@mask |= m
            #puts("%02d  %064b" % [index, @mask])
            set_index(parent(index), root) if root >= index && index > 0
            set_index(child0(index), root) if root <= index
            set_index(child1(index), root) if root <= index
        end
    end
    def unset_index(index, lvl, root = index)
        if index < 63 && set?(index)
            unset_index(child0(index), lvl+1, root) if root <= index
            unset_index(child1(index), lvl+1, root) if root <= index

            if (root >= index)
                if root == index || (!set?(child0(index)) && !set?(child1(index)))
                    @mask |= (1<<index)
                    unset_index(parent(index), lvl-1, root) if index > 0
                end
            else
                @mask |= (1<<index)
            end
        end
    end
    def set_size(bit, idx, value) @sizes[idx] = value end
    def unset_size(bit, idx) @sizes[idx] = nil end
end

class FastBitmap
    EMPTY = 0xFFFFFFFFFFFFFFFF
    attr_reader :mask, :sizes, :size_mask
    def initialize()
        @mask = EMPTY
        @sizes = Array.new(32)
        @size_mask = 0
    end
    def reset(value = EMPTY) @mask = value end
    def parent(index) (index - 1) / 2 end
    def child0(index) (2 * index) + 1 end
    def child1(index) (2 * index) + 2 end
    def set?(index) return (@mask & (1<<index)) == 0 end
    def set_index(index) @mask &= BIT_SETMASKS[index] end
    def unset_index(index, lvl)
        while true
            #puts("before = %064b" % @mask)
            #puts("bitmsk = %064b" % BIT_UNSETMASKS[index])
            @mask |= BIT_UNSETMASKS[index]
            #puts("after  = %064b" % @mask)
            sibbling = ((index & 1) == 0 ? index - 1 : index + 1)
            #puts("siblng = %064b" % (@mask & (1<<sibbling)))
            break if index == 0 || 0 == (@mask & (1<<sibbling))
            index = parent(index)
        end
    end
    def set_size(bit, idx, value)
        @sizes[idx] = value
        @size_mask |= (1<<bit)
    end
    def unset_size(bit, idx)
        @sizes[idx] = nil
        @size_mask &= ~(1<<bit)
    end
end

def allocate(bitmap, size)
    #puts "---------------\n ==> allocate(#{size})"
    lvl = miplevel(size)
    #puts "level = #{lvl}"
    msk = bitmap.mask & MIP_MASKS[lvl]
    #puts("msk = %016X" % msk)
    if msk != 0
        bit = firstbit(msk)
        #puts "bit = #{bit}"
        #puts "mipoffset = #{mipoffset(lvl)}"
        off = (bit - mipoffset(lvl))
        #puts "offset = #{off}"
        idx = (off * size) / (64*1024)
        #puts idx
        bitmap.set_index(bit)
        bitmap.set_size(bit, idx, size)
        puts("bitmap = %064b" % bitmap.mask)
        #puts(" --> 0x%08X" % (off * size))
        return off * size
    else
        return nil
    end
end

def deallocate(bitmap, ptr)
    unless ptr.nil?
        #puts("---------------\n <== deallocate(0x%08X)" % ptr)
        base = 0 # to replace with begin virtual address space reserved
        idx = (ptr - base) / (64 * 1024)
        sz = bitmap.sizes[idx]
        #puts "size = #{sz}"
        lvl = miplevel(sz)
        #puts "level = #{lvl}"
        off = ptr / sz
        #puts "off = #{off}"
        bit = off + mipoffset(lvl)
        bitmap.unset_index(bit, lvl)
        bitmap.unset_size(bit, idx)
    end
    puts("bitmap = %064b" % bitmap.mask)
end

def block_size2(bitmap, ptr)
    base = 0 # to replace with begin virtual address space reserved
    idx = (ptr - base) / (64 * 1024)
    msk = bitmap.size_mask & BIT_SIZEMASKS[idx]
    bit = firstbit(msk)
    lvl = floor_log2(bit + 1)
    return (2048 * 1024) / (1 << lvl)
end

def deallocate2(bitmap, ptr)
    unless ptr.nil?
        #puts("---------------\n <== deallocate(0x%08X)" % ptr)
        base = 0 # to replace with begin virtual address space reserved
        idx = (ptr - base) / (64 * 1024)
        msk = bitmap.size_mask & BIT_SIZEMASKS[idx]
        #puts("\n__> %064b\n==> %064b\n--> %064b" % [bitmap.size_mask, BIT_SIZEMASKS[idx], msk])
        bit = firstbit(msk)
        lvl = floor_log2(bit + 1)
        bitmap.unset_index(bit, lvl)
        bitmap.unset_size(bit, idx)
    end
    puts("bitmap = %064b" % bitmap.mask)
end

def size_in_bytes(bitmap)
    return popcnt(bitmap.mask & MIP_MASKS[5]) * (64 * 1024)
end

def test_bitmap(b2)
sz = 512 * 1024
puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, sz)
p0 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, sz)
p1 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, sz)
p2 = p
#puts(p.nil? ? "failed" : "succeed")
#puts "=============="
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, 64*1024)
p3 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
deallocate(b2, p0)
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, sz)
p0 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
deallocate(b2, p2)
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, sz)
p4 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
deallocate(b2, p0)
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
deallocate(b2, p1)
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, 1024*1024)
p5 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, 1024*1024)
p6 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, 64*1024)
p7 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
p = allocate(b2, 64*1024)
p8 = p
#puts(p.nil? ? "failed" : "succeed")
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
deallocate(b2, p5)
#puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
while size_in_bytes(b2) > 0
    p = allocate(b2, 64*1024)
    #puts(p.nil? ? "failed" : "succeed")
end
puts("size available = %8.4fkb" % (size_in_bytes(b2)/1024.0))
end

b = Bitmap.new()
nodes = Array.new(6).collect!{ [] }
b.each(0) do |i, depth|
    nodes[depth] << i
end
nodes.each do |indices|
    sz = (2048 / indices.length)
    mask = 0
    indices.each{ |i| mask |= 1<<i }
    puts("[%4d] %2d blocks : 0x%064bull : %2d = [ #{indices.join(", ")} ]" % [sz, indices.length, mask, firstbit(mask)])
end
=begin
b.set_index(1)
m = b.mask
b.reset
b.set_index(2)
m |= b.mask
puts("%064b" % m)
exit
=end
set_masks = Array.new(64, 0)
nodes.each do |indices|
    indices.each do |i|
        b.reset
        b.set_index(i)
        set_masks[i] = b.mask
        #puts("[%02d]  %064b" % [i, b.mask])
    end
end

puts "\nstatic constexpr u64 GMipmapLevelMasks[6] = {"
idx = 0
MIP_MASKS.each do |mask|
    puts("    0x%016Xull," % mask)
end
puts "};"

puts "\nstatic constexpr u64 GMipmapSetMasks[64] = {"
idx = 0
16.times do |y|
    line = "    "
    4.times do |x|
        m = set_masks[idx]
        #m = 0 if m.nil?
        line << ("0x%016Xull, " % m)
        idx += 1
    end
    puts line.rstrip
end
puts "};"

unset_masks = Array.new(64, 0)
nodes.each do |indices|
    indices.each do |i|
        b.reset(0)
        b.unset_index(i, 0)
        unset_masks[i] = b.mask
        #puts("[%02d]  %064b" % [i, b.mask])
    end
end

puts "\nstatic constexpr u64 GMipmapUnsetMasks[64] = {"
idx = 0
16.times do |y|
    line = "    "
    4.times do |x|
        m = unset_masks[idx]
        #m = 0 if m.nil?
        line << ("0x%016Xull, " % m)
        idx += 1
    end
    puts line.rstrip
end
puts "};"


size_masks = Array.new(32, 0)
nodes.each_with_index do |indices, lvl|
    indices.each do |bit|
        off = bit - mipoffset(lvl)
        idx = (off * MIP_SIZES[lvl]) / (64 * 1024)
        size_masks[idx] |= (1<<bit)
    end
end

puts "\nstatic constexpr u64 GMipmapSizeMasks[64] = {"
idx = 0
8.times do |y|
    line = "    "
    4.times do |x|
        m = size_masks[idx]
        #m = 0 if m.nil?
        line << ("0x%016Xull, " % m)
        idx += 1
    end
    puts line.rstrip
end
puts "};"

def test_allocate(size)
    #puts("test_allocate(%6d)" % size)
    print("[B1] %7.1fkb - " % (size_in_bytes($b1)/1024.0));p1 = allocate($b1, size)
    print("[B2] %7.1fkb - " % (size_in_bytes($b2)/1024.0));p2 = allocate($b2, size)
    raise "invalid alloc" if p1 != p2
    $allocs << [p1, size]
    #puts($b1.mask == $b2.mask ? "[Success]" : "[Failure]")
    raise "invalid bitmask" if $b1.mask != $b2.mask
end

def test_deallocate()
    p, size = *$allocs.shift
    #puts("test_deallocate(0x%08X)" % p)
    raise "invalid block size = #{size} != #{block_size2($b2, p)}" if block_size2($b2, p) != size
    print("[B1] %7.1fkb - " % (size_in_bytes($b1)/1024.0)); deallocate($b1, p)
    print("[B2] %7.1fkb - " % (size_in_bytes($b2)/1024.0)); deallocate2($b2, p)
    #puts($b1.mask == $b2.mask ? "[Success]" : "[Failure]")
    raise "invalid bitmask" if $b1.mask != $b2.mask
end

#srand(6969)

1.times do

$b1 = Bitmap.new
#test_bitmap(b1)
$b2 = FastBitmap.new
#test_bitmap(b2)

$allocs = []

puts "============================"
$blocks = [
    64*1024,
    64*1024,
    128*1024,
    256*1024,
    64*1024,
    128*1024,
    64*1024,
    64*1024,
    128*1024,
    128*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
    64*1024,
]
$blocks.shuffle!

$blocks.each do |sz|
    test_allocate(sz)
end

puts "============================"
$allocs.shuffle!

$allocs.length.times do
    test_deallocate()
end

end

#pp $b1
#pp $b2