
// Contains every shuffle combinaitions for dimension 2, 3 & 4
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE2()
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE3()
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE4()

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE2(_Macro, ...) \
    COMMA_PROTECT(_Macro(ww,   3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wx,   3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wy,   3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wz,   3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xw,   0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xx,   0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xy,   0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xz,   0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yw,   1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yx,   1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yy,   1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yz,   1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zw,   2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zx,   2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zy,   2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zz,   2, 2, ##__VA_ARGS__))

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE3(_Macro, ...) \
    COMMA_PROTECT(_Macro(www,  3, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwx,  3, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwy,  3, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwz,  3, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxw,  3, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxx,  3, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxy,  3, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxz,  3, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyw,  3, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyx,  3, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyy,  3, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyz,  3, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzw,  3, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzx,  3, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzy,  3, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzz,  3, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xww,  0, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwx,  0, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwy,  0, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwz,  0, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxw,  0, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxx,  0, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxy,  0, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxz,  0, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyw,  0, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyx,  0, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyy,  0, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyz,  0, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzw,  0, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzx,  0, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzy,  0, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzz,  0, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yww,  1, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywx,  1, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywy,  1, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywz,  1, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxw,  1, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxx,  1, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxy,  1, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxz,  1, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyw,  1, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyx,  1, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyy,  1, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyz,  1, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzw,  1, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzx,  1, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzy,  1, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzz,  1, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zww,  2, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwx,  2, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwy,  2, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwz,  2, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxw,  2, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxx,  2, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxy,  2, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxz,  2, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyw,  2, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyx,  2, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyy,  2, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyz,  2, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzw,  2, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzx,  2, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzy,  2, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzz,  2, 2, 2, ##__VA_ARGS__))

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE4(_Macro, ...) \
    COMMA_PROTECT(_Macro(wwww, 3, 3, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwwx, 3, 3, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwwy, 3, 3, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwwz, 3, 3, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwxw, 3, 3, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwxx, 3, 3, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwxy, 3, 3, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwxz, 3, 3, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwyw, 3, 3, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwyx, 3, 3, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwyy, 3, 3, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwyz, 3, 3, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwzw, 3, 3, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwzx, 3, 3, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwzy, 3, 3, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wwzz, 3, 3, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxww, 3, 0, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxwx, 3, 0, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxwy, 3, 0, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxwz, 3, 0, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxxw, 3, 0, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxxx, 3, 0, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxxy, 3, 0, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxxz, 3, 0, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxyw, 3, 0, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxyx, 3, 0, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxyy, 3, 0, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxyz, 3, 0, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxzw, 3, 0, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxzx, 3, 0, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxzy, 3, 0, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wxzz, 3, 0, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyww, 3, 1, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wywx, 3, 1, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wywy, 3, 1, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wywz, 3, 1, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyxw, 3, 1, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyxx, 3, 1, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyxy, 3, 1, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyxz, 3, 1, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyyw, 3, 1, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyyx, 3, 1, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyyy, 3, 1, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyyz, 3, 1, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyzw, 3, 1, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyzx, 3, 1, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyzy, 3, 1, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wyzz, 3, 1, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzww, 3, 2, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzwx, 3, 2, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzwy, 3, 2, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzwz, 3, 2, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzxw, 3, 2, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzxx, 3, 2, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzxy, 3, 2, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzxz, 3, 2, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzyw, 3, 2, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzyx, 3, 2, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzyy, 3, 2, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzyz, 3, 2, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzzw, 3, 2, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzzx, 3, 2, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzzy, 3, 2, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(wzzz, 3, 2, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwww, 0, 3, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwwx, 0, 3, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwwy, 0, 3, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwwz, 0, 3, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwxw, 0, 3, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwxx, 0, 3, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwxy, 0, 3, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwxz, 0, 3, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwyw, 0, 3, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwyx, 0, 3, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwyy, 0, 3, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwyz, 0, 3, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwzw, 0, 3, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwzx, 0, 3, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwzy, 0, 3, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xwzz, 0, 3, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxww, 0, 0, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxwx, 0, 0, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxwy, 0, 0, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxwz, 0, 0, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxxw, 0, 0, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxxx, 0, 0, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxxy, 0, 0, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxxz, 0, 0, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxyw, 0, 0, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxyx, 0, 0, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxyy, 0, 0, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxyz, 0, 0, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxzw, 0, 0, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxzx, 0, 0, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxzy, 0, 0, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xxzz, 0, 0, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyww, 0, 1, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xywx, 0, 1, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xywy, 0, 1, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xywz, 0, 1, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyxw, 0, 1, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyxx, 0, 1, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyxy, 0, 1, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyxz, 0, 1, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyyw, 0, 1, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyyx, 0, 1, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyyy, 0, 1, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyyz, 0, 1, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyzw, 0, 1, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyzx, 0, 1, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyzy, 0, 1, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xyzz, 0, 1, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzww, 0, 2, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzwx, 0, 2, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzwy, 0, 2, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzwz, 0, 2, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzxw, 0, 2, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzxx, 0, 2, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzxy, 0, 2, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzxz, 0, 2, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzyw, 0, 2, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzyx, 0, 2, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzyy, 0, 2, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzyz, 0, 2, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzzw, 0, 2, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzzx, 0, 2, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzzy, 0, 2, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(xzzz, 0, 2, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywww, 1, 3, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywwx, 1, 3, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywwy, 1, 3, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywwz, 1, 3, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywxw, 1, 3, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywxx, 1, 3, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywxy, 1, 3, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywxz, 1, 3, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywyw, 1, 3, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywyx, 1, 3, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywyy, 1, 3, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywyz, 1, 3, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywzw, 1, 3, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywzx, 1, 3, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywzy, 1, 3, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ywzz, 1, 3, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxww, 1, 0, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxwx, 1, 0, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxwy, 1, 0, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxwz, 1, 0, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxxw, 1, 0, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxxx, 1, 0, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxxy, 1, 0, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxxz, 1, 0, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxyw, 1, 0, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxyx, 1, 0, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxyy, 1, 0, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxyz, 1, 0, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxzw, 1, 0, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxzx, 1, 0, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxzy, 1, 0, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yxzz, 1, 0, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyww, 1, 1, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yywx, 1, 1, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yywy, 1, 1, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yywz, 1, 1, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyxw, 1, 1, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyxx, 1, 1, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyxy, 1, 1, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyxz, 1, 1, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyyw, 1, 1, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyyx, 1, 1, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyyy, 1, 1, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyyz, 1, 1, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyzw, 1, 1, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyzx, 1, 1, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyzy, 1, 1, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yyzz, 1, 1, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzww, 1, 2, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzwx, 1, 2, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzwy, 1, 2, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzwz, 1, 2, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzxw, 1, 2, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzxx, 1, 2, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzxy, 1, 2, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzxz, 1, 2, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzyw, 1, 2, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzyx, 1, 2, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzyy, 1, 2, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzyz, 1, 2, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzzw, 1, 2, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzzx, 1, 2, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzzy, 1, 2, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(yzzz, 1, 2, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwww, 2, 3, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwwx, 2, 3, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwwy, 2, 3, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwwz, 2, 3, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwxw, 2, 3, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwxx, 2, 3, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwxy, 2, 3, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwxz, 2, 3, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwyw, 2, 3, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwyx, 2, 3, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwyy, 2, 3, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwyz, 2, 3, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwzw, 2, 3, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwzx, 2, 3, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwzy, 2, 3, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zwzz, 2, 3, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxww, 2, 0, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxwx, 2, 0, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxwy, 2, 0, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxwz, 2, 0, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxxw, 2, 0, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxxx, 2, 0, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxxy, 2, 0, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxxz, 2, 0, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxyw, 2, 0, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxyx, 2, 0, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxyy, 2, 0, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxyz, 2, 0, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxzw, 2, 0, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxzx, 2, 0, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxzy, 2, 0, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zxzz, 2, 0, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyww, 2, 1, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zywx, 2, 1, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zywy, 2, 1, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zywz, 2, 1, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyxw, 2, 1, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyxx, 2, 1, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyxy, 2, 1, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyxz, 2, 1, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyyw, 2, 1, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyyx, 2, 1, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyyy, 2, 1, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyyz, 2, 1, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyzw, 2, 1, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyzx, 2, 1, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyzy, 2, 1, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zyzz, 2, 1, 2, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzww, 2, 2, 3, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzwx, 2, 2, 3, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzwy, 2, 2, 3, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzwz, 2, 2, 3, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzxw, 2, 2, 0, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzxx, 2, 2, 0, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzxy, 2, 2, 0, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzxz, 2, 2, 0, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzyw, 2, 2, 1, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzyx, 2, 2, 1, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzyy, 2, 2, 1, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzyz, 2, 2, 1, 2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzzw, 2, 2, 2, 3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzzx, 2, 2, 2, 0, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzzy, 2, 2, 2, 1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(zzzz, 2, 2, 2, 2, ##__VA_ARGS__))