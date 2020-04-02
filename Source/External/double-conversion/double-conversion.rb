# frozen_string_literal: true

$Build.ppe_external!('double-conversion') do
    force_includes!(File.join(abs_path, 'Public', 'double-conversion-external.h'))
    extra_files!(*%w{
        Public/double-conversion-external.h
        double-conversion.git/double-conversion/bignum.h
        double-conversion.git/double-conversion/bignum-dtoa.h
        double-conversion.git/double-conversion/cached-powers.h
        double-conversion.git/double-conversion/diy-fp.h
        double-conversion.git/double-conversion/double-conversion.h
        double-conversion.git/double-conversion/double-to-string.h
        double-conversion.git/double-conversion/fast-dtoa.h
        double-conversion.git/double-conversion/fixed-dtoa.h
        double-conversion.git/double-conversion/ieee.h
        double-conversion.git/double-conversion/string-to-double.h
        double-conversion.git/double-conversion/strtod.h
        double-conversion.git/double-conversion/utils.h })
    isolated_files!(*%w{
        double-conversion.git/double-conversion/bignum.cc
        double-conversion.git/double-conversion/bignum-dtoa.cc
        double-conversion.git/double-conversion/cached-powers.cc
        double-conversion.git/double-conversion/double-to-string.cc
        double-conversion.git/double-conversion/fast-dtoa.cc
        double-conversion.git/double-conversion/fixed-dtoa.cc
        double-conversion.git/double-conversion/string-to-double.cc
        double-conversion.git/double-conversion/strtod.cc })
end
