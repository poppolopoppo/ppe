//----------------------------------------------------------------------------
namespace Time {
    UNITS_DECL(Microseconds, _Tag, 1000, void);
    UNITS_DECL(Milliseconds, _Tag, 1000, Microseconds);
    UNITS_DECL(Seconds, _Tag, 60, Milliseconds);
    UNITS_DECL(Minutes, _Tag, 60, Seconds);
    UNITS_DECL(Hours, _Tag, 24, Minutes);
    UNITS_DECL(Days, _Tag, 1, Hours);
}
//----------------------------------------------------------------------------
namespace Distance {
    UNITS_DECL(Millimeters, _Tag, 10, void);
    UNITS_DECL(Centimeters, _Tag, 100, Millimeters);
    UNITS_DECL(Meters, _Tag, 1000, Centimeters);
    UNITS_DECL(Kilometers, _Tag, 1, Meters);
}
//----------------------------------------------------------------------------
namespace Mass {
    UNITS_DECL(Picograms, _Tag, 1000, void);
    UNITS_DECL(Nanograms, _Tag, 1000, Picograms);
    UNITS_DECL(Micrograms, _Tag, 1000, Nanograms);
    UNITS_DECL(Milligrams, _Tag, 1000, Micrograms);
    UNITS_DECL(Grams, _Tag, 1000, Milligrams);
    UNITS_DECL(Kilograms, _Tag, 1000, Grams);
    UNITS_DECL(Tonnes, _Tag, 1, Kilograms);
}
//----------------------------------------------------------------------------
namespace Storage {
    UNITS_DECL(Bytes, _Tag, 1024, void);
    UNITS_DECL(Kilobytes, _Tag, 1024, Bytes);
    UNITS_DECL(Megabytes, _Tag, 1024, Kilobytes);
    UNITS_DECL(Gigabytes, _Tag, 1024, Megabytes);
    UNITS_DECL(Terabytes, _Tag, 1024, Gigabytes);
    UNITS_DECL(Petabytes, _Tag, 1, Terabytes);
}
//----------------------------------------------------------------------------
