#ifndef DEF_SCALARVECTOR_SHUFFLE0
#   define DEF_SCALARVECTOR_SHUFFLE0(_Alias, _Offset, _N)
#endif
#ifndef DEF_SCALARVECTOR_SHUFFLE2
#   define DEF_SCALARVECTOR_SHUFFLE2(_Alias, _0, _1)
#endif
#ifndef DEF_SCALARVECTOR_SHUFFLE3
#   define DEF_SCALARVECTOR_SHUFFLE3(_Alias, _0, _1, _2)
#endif
#ifndef DEF_SCALARVECTOR_SHUFFLE4
#   define DEF_SCALARVECTOR_SHUFFLE4(_Alias, _0, _1, _2, _3)
#endif

DEF_SCALARVECTOR_SHUFFLE0(  xy, 0, 2)
DEF_SCALARVECTOR_SHUFFLE0( xyz, 0, 3)
DEF_SCALARVECTOR_SHUFFLE0(xyzw, 0, 4)
DEF_SCALARVECTOR_SHUFFLE0(  yz, 1, 2)
DEF_SCALARVECTOR_SHUFFLE0( yzw, 1, 3)
DEF_SCALARVECTOR_SHUFFLE0(  zw, 2, 2)
DEF_SCALARVECTOR_SHUFFLE2(  xx, 0, 0)
DEF_SCALARVECTOR_SHUFFLE2(  xz, 0, 2)
DEF_SCALARVECTOR_SHUFFLE2(  xw, 0, 3)
DEF_SCALARVECTOR_SHUFFLE2(  yx, 1, 0)
DEF_SCALARVECTOR_SHUFFLE2(  yy, 1, 1)
DEF_SCALARVECTOR_SHUFFLE2(  yw, 1, 3)
DEF_SCALARVECTOR_SHUFFLE2(  zx, 2, 0)
DEF_SCALARVECTOR_SHUFFLE2(  zy, 2, 1)
DEF_SCALARVECTOR_SHUFFLE2(  zz, 2, 2)
DEF_SCALARVECTOR_SHUFFLE2(  wx, 3, 0)
DEF_SCALARVECTOR_SHUFFLE2(  wy, 3, 1)
DEF_SCALARVECTOR_SHUFFLE2(  wz, 3, 2)
DEF_SCALARVECTOR_SHUFFLE2(  ww, 3, 3)
DEF_SCALARVECTOR_SHUFFLE3( xxx, 0, 0, 0)
DEF_SCALARVECTOR_SHUFFLE3( xxy, 0, 0, 1)
DEF_SCALARVECTOR_SHUFFLE3( xxz, 0, 0, 2)
DEF_SCALARVECTOR_SHUFFLE3( xxw, 0, 0, 3)
DEF_SCALARVECTOR_SHUFFLE3( xyx, 0, 1, 0)
DEF_SCALARVECTOR_SHUFFLE3( xyy, 0, 1, 1)
DEF_SCALARVECTOR_SHUFFLE3( xyw, 0, 1, 3)
DEF_SCALARVECTOR_SHUFFLE3( xzx, 0, 2, 0)
DEF_SCALARVECTOR_SHUFFLE3( xzy, 0, 2, 1)
DEF_SCALARVECTOR_SHUFFLE3( xzz, 0, 2, 2)
DEF_SCALARVECTOR_SHUFFLE3( xzw, 0, 2, 3)
DEF_SCALARVECTOR_SHUFFLE3( xwx, 0, 3, 0)
DEF_SCALARVECTOR_SHUFFLE3( xwy, 0, 3, 1)
DEF_SCALARVECTOR_SHUFFLE3( xwz, 0, 3, 2)
DEF_SCALARVECTOR_SHUFFLE3( xww, 0, 3, 3)
DEF_SCALARVECTOR_SHUFFLE3( yxx, 1, 0, 0)
DEF_SCALARVECTOR_SHUFFLE3( yxy, 1, 0, 1)
DEF_SCALARVECTOR_SHUFFLE3( yxz, 1, 0, 2)
DEF_SCALARVECTOR_SHUFFLE3( yxw, 1, 0, 3)
DEF_SCALARVECTOR_SHUFFLE3( yyx, 1, 1, 0)
DEF_SCALARVECTOR_SHUFFLE3( yyy, 1, 1, 1)
DEF_SCALARVECTOR_SHUFFLE3( yyz, 1, 1, 2)
DEF_SCALARVECTOR_SHUFFLE3( yyw, 1, 1, 3)
DEF_SCALARVECTOR_SHUFFLE3( yzx, 1, 2, 0)
DEF_SCALARVECTOR_SHUFFLE3( yzy, 1, 2, 1)
DEF_SCALARVECTOR_SHUFFLE3( yzz, 1, 2, 2)
DEF_SCALARVECTOR_SHUFFLE3( ywx, 1, 3, 0)
DEF_SCALARVECTOR_SHUFFLE3( ywy, 1, 3, 1)
DEF_SCALARVECTOR_SHUFFLE3( ywz, 1, 3, 2)
DEF_SCALARVECTOR_SHUFFLE3( yww, 1, 3, 3)
DEF_SCALARVECTOR_SHUFFLE3( zxx, 2, 0, 0)
DEF_SCALARVECTOR_SHUFFLE3( zxy, 2, 0, 1)
DEF_SCALARVECTOR_SHUFFLE3( zxz, 2, 0, 2)
DEF_SCALARVECTOR_SHUFFLE3( zxw, 2, 0, 3)
DEF_SCALARVECTOR_SHUFFLE3( zyx, 2, 1, 0)
DEF_SCALARVECTOR_SHUFFLE3( zyy, 2, 1, 1)
DEF_SCALARVECTOR_SHUFFLE3( zyz, 2, 1, 2)
DEF_SCALARVECTOR_SHUFFLE3( zyw, 2, 1, 3)
DEF_SCALARVECTOR_SHUFFLE3( zzx, 2, 2, 0)
DEF_SCALARVECTOR_SHUFFLE3( zzy, 2, 2, 1)
DEF_SCALARVECTOR_SHUFFLE3( zzz, 2, 2, 2)
DEF_SCALARVECTOR_SHUFFLE3( zzw, 2, 2, 3)
DEF_SCALARVECTOR_SHUFFLE3( zwx, 2, 3, 0)
DEF_SCALARVECTOR_SHUFFLE3( zwy, 2, 3, 1)
DEF_SCALARVECTOR_SHUFFLE3( zwz, 2, 3, 2)
DEF_SCALARVECTOR_SHUFFLE3( zww, 2, 3, 3)
DEF_SCALARVECTOR_SHUFFLE3( wxx, 3, 0, 0)
DEF_SCALARVECTOR_SHUFFLE3( wxy, 3, 0, 1)
DEF_SCALARVECTOR_SHUFFLE3( wxz, 3, 0, 2)
DEF_SCALARVECTOR_SHUFFLE3( wxw, 3, 0, 3)
DEF_SCALARVECTOR_SHUFFLE3( wyx, 3, 1, 0)
DEF_SCALARVECTOR_SHUFFLE3( wyy, 3, 1, 1)
DEF_SCALARVECTOR_SHUFFLE3( wyz, 3, 1, 2)
DEF_SCALARVECTOR_SHUFFLE3( wyw, 3, 1, 3)
DEF_SCALARVECTOR_SHUFFLE3( wzx, 3, 2, 0)
DEF_SCALARVECTOR_SHUFFLE3( wzy, 3, 2, 1)
DEF_SCALARVECTOR_SHUFFLE3( wzz, 3, 2, 2)
DEF_SCALARVECTOR_SHUFFLE3( wzw, 3, 2, 3)
DEF_SCALARVECTOR_SHUFFLE3( wwx, 3, 3, 0)
DEF_SCALARVECTOR_SHUFFLE3( wwy, 3, 3, 1)
DEF_SCALARVECTOR_SHUFFLE3( wwz, 3, 3, 2)
DEF_SCALARVECTOR_SHUFFLE3( www, 3, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(xxxx, 0, 0, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(xxxy, 0, 0, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(xxxz, 0, 0, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(xxxw, 0, 0, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(xxyx, 0, 0, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(xxyy, 0, 0, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(xxyz, 0, 0, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(xxyw, 0, 0, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(xxzx, 0, 0, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(xxzy, 0, 0, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(xxzz, 0, 0, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(xxzw, 0, 0, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(xxwx, 0, 0, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(xxwy, 0, 0, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(xxwz, 0, 0, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(xxww, 0, 0, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(xyxx, 0, 1, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(xyxy, 0, 1, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(xyxz, 0, 1, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(xyxw, 0, 1, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(xyyx, 0, 1, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(xyyy, 0, 1, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(xyyz, 0, 1, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(xyyw, 0, 1, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(xyzx, 0, 1, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(xyzy, 0, 1, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(xyzz, 0, 1, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(xywx, 0, 1, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(xywy, 0, 1, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(xywz, 0, 1, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(xyww, 0, 1, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(xzxx, 0, 2, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(xzxy, 0, 2, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(xzxz, 0, 2, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(xzxw, 0, 2, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(xzyx, 0, 2, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(xzyy, 0, 2, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(xzyz, 0, 2, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(xzyw, 0, 2, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(xzzx, 0, 2, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(xzzy, 0, 2, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(xzzz, 0, 2, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(xzzw, 0, 2, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(xzwx, 0, 2, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(xzwy, 0, 2, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(xzwz, 0, 2, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(xzww, 0, 2, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(xwxx, 0, 3, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(xwxy, 0, 3, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(xwxz, 0, 3, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(xwxw, 0, 3, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(xwyx, 0, 3, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(xwyy, 0, 3, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(xwyz, 0, 3, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(xwyw, 0, 3, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(xwzx, 0, 3, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(xwzy, 0, 3, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(xwzz, 0, 3, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(xwzw, 0, 3, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(xwwx, 0, 3, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(xwwy, 0, 3, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(xwwz, 0, 3, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(xwww, 0, 3, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(yxxx, 1, 0, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(yxxy, 1, 0, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(yxxz, 1, 0, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(yxxw, 1, 0, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(yxyx, 1, 0, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(yxyy, 1, 0, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(yxyz, 1, 0, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(yxyw, 1, 0, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(yxzx, 1, 0, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(yxzy, 1, 0, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(yxzz, 1, 0, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(yxzw, 1, 0, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(yxwx, 1, 0, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(yxwy, 1, 0, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(yxwz, 1, 0, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(yxww, 1, 0, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(yyxx, 1, 1, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(yyxy, 1, 1, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(yyxz, 1, 1, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(yyxw, 1, 1, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(yyyx, 1, 1, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(yyyy, 1, 1, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(yyyz, 1, 1, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(yyyw, 1, 1, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(yyzx, 1, 1, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(yyzy, 1, 1, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(yyzz, 1, 1, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(yyzw, 1, 1, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(yywx, 1, 1, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(yywy, 1, 1, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(yywz, 1, 1, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(yyww, 1, 1, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(yzxx, 1, 2, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(yzxy, 1, 2, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(yzxz, 1, 2, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(yzxw, 1, 2, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(yzyx, 1, 2, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(yzyy, 1, 2, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(yzyz, 1, 2, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(yzyw, 1, 2, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(yzzx, 1, 2, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(yzzy, 1, 2, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(yzzz, 1, 2, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(yzzw, 1, 2, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(yzwx, 1, 2, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(yzwy, 1, 2, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(yzwz, 1, 2, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(yzww, 1, 2, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(ywxx, 1, 3, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(ywxy, 1, 3, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(ywxz, 1, 3, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(ywxw, 1, 3, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(ywyx, 1, 3, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(ywyy, 1, 3, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(ywyz, 1, 3, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(ywyw, 1, 3, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(ywzx, 1, 3, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(ywzy, 1, 3, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(ywzz, 1, 3, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(ywzw, 1, 3, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(ywwx, 1, 3, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(ywwy, 1, 3, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(ywwz, 1, 3, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(ywww, 1, 3, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(zxxx, 2, 0, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(zxxy, 2, 0, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(zxxz, 2, 0, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(zxxw, 2, 0, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(zxyx, 2, 0, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(zxyy, 2, 0, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(zxyz, 2, 0, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(zxyw, 2, 0, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(zxzx, 2, 0, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(zxzy, 2, 0, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(zxzz, 2, 0, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(zxzw, 2, 0, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(zxwx, 2, 0, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(zxwy, 2, 0, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(zxwz, 2, 0, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(zxww, 2, 0, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(zyxx, 2, 1, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(zyxy, 2, 1, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(zyxz, 2, 1, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(zyxw, 2, 1, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(zyyx, 2, 1, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(zyyy, 2, 1, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(zyyz, 2, 1, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(zyyw, 2, 1, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(zyzx, 2, 1, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(zyzy, 2, 1, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(zyzz, 2, 1, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(zyzw, 2, 1, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(zywx, 2, 1, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(zywy, 2, 1, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(zywz, 2, 1, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(zyww, 2, 1, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(zzxx, 2, 2, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(zzxy, 2, 2, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(zzxz, 2, 2, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(zzxw, 2, 2, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(zzyx, 2, 2, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(zzyy, 2, 2, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(zzyz, 2, 2, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(zzyw, 2, 2, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(zzzx, 2, 2, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(zzzy, 2, 2, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(zzzz, 2, 2, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(zzzw, 2, 2, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(zzwx, 2, 2, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(zzwy, 2, 2, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(zzwz, 2, 2, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(zzww, 2, 2, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(zwxx, 2, 3, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(zwxy, 2, 3, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(zwxz, 2, 3, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(zwxw, 2, 3, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(zwyx, 2, 3, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(zwyy, 2, 3, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(zwyz, 2, 3, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(zwyw, 2, 3, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(zwzx, 2, 3, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(zwzy, 2, 3, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(zwzz, 2, 3, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(zwzw, 2, 3, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(zwwx, 2, 3, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(zwwy, 2, 3, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(zwwz, 2, 3, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(zwww, 2, 3, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(wxxx, 3, 0, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(wxxy, 3, 0, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(wxxz, 3, 0, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(wxxw, 3, 0, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(wxyx, 3, 0, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(wxyy, 3, 0, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(wxyz, 3, 0, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(wxyw, 3, 0, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(wxzx, 3, 0, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(wxzy, 3, 0, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(wxzz, 3, 0, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(wxzw, 3, 0, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(wxwx, 3, 0, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(wxwy, 3, 0, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(wxwz, 3, 0, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(wxww, 3, 0, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(wyxx, 3, 1, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(wyxy, 3, 1, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(wyxz, 3, 1, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(wyxw, 3, 1, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(wyyx, 3, 1, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(wyyy, 3, 1, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(wyyz, 3, 1, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(wyyw, 3, 1, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(wyzx, 3, 1, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(wyzy, 3, 1, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(wyzz, 3, 1, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(wyzw, 3, 1, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(wywx, 3, 1, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(wywy, 3, 1, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(wywz, 3, 1, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(wyww, 3, 1, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(wzxx, 3, 2, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(wzxy, 3, 2, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(wzxz, 3, 2, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(wzxw, 3, 2, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(wzyx, 3, 2, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(wzyy, 3, 2, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(wzyz, 3, 2, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(wzyw, 3, 2, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(wzzx, 3, 2, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(wzzy, 3, 2, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(wzzz, 3, 2, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(wzzw, 3, 2, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(wzwx, 3, 2, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(wzwy, 3, 2, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(wzwz, 3, 2, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(wzww, 3, 2, 3, 3)
DEF_SCALARVECTOR_SHUFFLE4(wwxx, 3, 3, 0, 0)
DEF_SCALARVECTOR_SHUFFLE4(wwxy, 3, 3, 0, 1)
DEF_SCALARVECTOR_SHUFFLE4(wwxz, 3, 3, 0, 2)
DEF_SCALARVECTOR_SHUFFLE4(wwxw, 3, 3, 0, 3)
DEF_SCALARVECTOR_SHUFFLE4(wwyx, 3, 3, 1, 0)
DEF_SCALARVECTOR_SHUFFLE4(wwyy, 3, 3, 1, 1)
DEF_SCALARVECTOR_SHUFFLE4(wwyz, 3, 3, 1, 2)
DEF_SCALARVECTOR_SHUFFLE4(wwyw, 3, 3, 1, 3)
DEF_SCALARVECTOR_SHUFFLE4(wwzx, 3, 3, 2, 0)
DEF_SCALARVECTOR_SHUFFLE4(wwzy, 3, 3, 2, 1)
DEF_SCALARVECTOR_SHUFFLE4(wwzz, 3, 3, 2, 2)
DEF_SCALARVECTOR_SHUFFLE4(wwzw, 3, 3, 2, 3)
DEF_SCALARVECTOR_SHUFFLE4(wwwx, 3, 3, 3, 0)
DEF_SCALARVECTOR_SHUFFLE4(wwwy, 3, 3, 3, 1)
DEF_SCALARVECTOR_SHUFFLE4(wwwz, 3, 3, 3, 2)
DEF_SCALARVECTOR_SHUFFLE4(wwww, 3, 3, 3, 3)