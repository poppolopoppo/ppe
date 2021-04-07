# frozen_string_literal: true

# https://github.com/rails/rails/blob/master/activesupport/lib/active_support/core_ext/object/deep_dup.rb

class Object
    # Returns a deep copy of object if it's duplicable. If it's
    # not duplicable, returns +self+.
    #
    #   object = Object.new
    #   dup    = object.deep_dup
    #   dup.instance_variable_set(:@a, 1)
    #
    #   object.instance_variable_defined?(:@a) # => false
    #   dup.instance_variable_defined?(:@a)    # => true
    def deep_dup
        obj = dup
        obj.instance_variables.each do |name|
            value = obj.instance_variable_get(name)
            obj.instance_variable_set(name, value.deep_dup)
        end
        return obj
    end
end

module Kernel_deep_dup
    def deep_dup()
        return self.dup
    end
end

class Integer
    extend Kernel_deep_dup
end
class Float
    extend Kernel_deep_dup
end
class String
    extend Kernel_deep_dup
end
class Symbol
    extend Kernel_deep_dup
end

class Array
    # Returns a deep copy of array.
    #
    #   array = [1, [2, 3]]
    #   dup   = array.deep_dup
    #   dup[1][2] = 4
    #
    #   array[1][2] # => nil
    #   dup[1][2]   # => 4
    def deep_dup
        return map(&:deep_dup)
    end
end

class Hash
    # Returns a deep copy of hash.
    #
    #   hash = { a: { b: 'b' } }
    #   dup  = hash.deep_dup
    #   dup[:a][:c] = 'c'
    #
    #   hash[:a][:c] # => nil
    #   dup[:a][:c]  # => "c"
    def deep_dup
        hash = self.dup
        each_pair do |key, value|
            if key.frozen? && ::String === key
                hash[key] = value.deep_dup
            else
                hash.delete(key)
                hash[key.deep_dup] = value.deep_dup
            end
        end
        return hash
    end
end