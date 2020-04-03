# frozen_string_literal: true

$Build.ppe_headers!(:stb) do
    extra_files!(*%w{
        stb.git/stb_dxt.h
        stb.git/stb_image.h
        stb.git/stb_image_resize.h
        stb.git/stb_image_write.h })
end
