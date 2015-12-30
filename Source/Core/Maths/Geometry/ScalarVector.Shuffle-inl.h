
// Contains every shuffle combinaitions for dimension 2, 3 & 4
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE1()
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE2()
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE3()
//  FOREACH_CORE_SCALARVECTOR_SHUFFLE4()

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE1(_Macro) \
  _Macro(2, xx, 0, 0) \
  _Macro(3, xxx, 0, 0, 0) \
  _Macro(4, xxxx, 0, 0, 0, 0)

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE2(_Macro) \
  _Macro(2, xy, 0, 1) \
  _Macro(2, yx, 1, 0) \
  _Macro(2, yy, 1, 1) \
  _Macro(3, xxy, 0, 0, 1) \
  _Macro(3, xyx, 0, 1, 0) \
  _Macro(3, xyy, 0, 1, 1) \
  _Macro(3, yxx, 1, 0, 0) \
  _Macro(3, yxy, 1, 0, 1) \
  _Macro(3, yyx, 1, 1, 0) \
  _Macro(3, yyy, 1, 1, 1) \
  _Macro(4, xxxy, 0, 0, 0, 1) \
  _Macro(4, xxyx, 0, 0, 1, 0) \
  _Macro(4, xxyy, 0, 0, 1, 1) \
  _Macro(4, xyxx, 0, 1, 0, 0) \
  _Macro(4, xyxy, 0, 1, 0, 1) \
  _Macro(4, xyyx, 0, 1, 1, 0) \
  _Macro(4, xyyy, 0, 1, 1, 1) \
  _Macro(4, yxxx, 1, 0, 0, 0) \
  _Macro(4, yxxy, 1, 0, 0, 1) \
  _Macro(4, yxyx, 1, 0, 1, 0) \
  _Macro(4, yxyy, 1, 0, 1, 1) \
  _Macro(4, yyxx, 1, 1, 0, 0) \
  _Macro(4, yyxy, 1, 1, 0, 1) \
  _Macro(4, yyyx, 1, 1, 1, 0) \
  _Macro(4, yyyy, 1, 1, 1, 1)

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE3(_Macro) \
  _Macro(2, xz, 0, 2) \
  _Macro(2, yz, 1, 2) \
  _Macro(2, zx, 2, 0) \
  _Macro(2, zy, 2, 1) \
  _Macro(2, zz, 2, 2) \
  _Macro(3, xxz, 0, 0, 2) \
  _Macro(3, xyz, 0, 1, 2) \
  _Macro(3, xzx, 0, 2, 0) \
  _Macro(3, xzy, 0, 2, 1) \
  _Macro(3, xzz, 0, 2, 2) \
  _Macro(3, yxz, 1, 0, 2) \
  _Macro(3, yyz, 1, 1, 2) \
  _Macro(3, yzx, 1, 2, 0) \
  _Macro(3, yzy, 1, 2, 1) \
  _Macro(3, yzz, 1, 2, 2) \
  _Macro(3, zxx, 2, 0, 0) \
  _Macro(3, zxy, 2, 0, 1) \
  _Macro(3, zxz, 2, 0, 2) \
  _Macro(3, zyx, 2, 1, 0) \
  _Macro(3, zyy, 2, 1, 1) \
  _Macro(3, zyz, 2, 1, 2) \
  _Macro(3, zzx, 2, 2, 0) \
  _Macro(3, zzy, 2, 2, 1) \
  _Macro(3, zzz, 2, 2, 2) \
  _Macro(4, xxxz, 0, 0, 0, 2) \
  _Macro(4, xxyz, 0, 0, 1, 2) \
  _Macro(4, xxzx, 0, 0, 2, 0) \
  _Macro(4, xxzy, 0, 0, 2, 1) \
  _Macro(4, xxzz, 0, 0, 2, 2) \
  _Macro(4, xyxz, 0, 1, 0, 2) \
  _Macro(4, xyyz, 0, 1, 1, 2) \
  _Macro(4, xyzx, 0, 1, 2, 0) \
  _Macro(4, xyzy, 0, 1, 2, 1) \
  _Macro(4, xyzz, 0, 1, 2, 2) \
  _Macro(4, xzxx, 0, 2, 0, 0) \
  _Macro(4, xzxy, 0, 2, 0, 1) \
  _Macro(4, xzxz, 0, 2, 0, 2) \
  _Macro(4, xzyx, 0, 2, 1, 0) \
  _Macro(4, xzyy, 0, 2, 1, 1) \
  _Macro(4, xzyz, 0, 2, 1, 2) \
  _Macro(4, xzzx, 0, 2, 2, 0) \
  _Macro(4, xzzy, 0, 2, 2, 1) \
  _Macro(4, xzzz, 0, 2, 2, 2) \
  _Macro(4, yxxz, 1, 0, 0, 2) \
  _Macro(4, yxyz, 1, 0, 1, 2) \
  _Macro(4, yxzx, 1, 0, 2, 0) \
  _Macro(4, yxzy, 1, 0, 2, 1) \
  _Macro(4, yxzz, 1, 0, 2, 2) \
  _Macro(4, yyxz, 1, 1, 0, 2) \
  _Macro(4, yyyz, 1, 1, 1, 2) \
  _Macro(4, yyzx, 1, 1, 2, 0) \
  _Macro(4, yyzy, 1, 1, 2, 1) \
  _Macro(4, yyzz, 1, 1, 2, 2) \
  _Macro(4, yzxx, 1, 2, 0, 0) \
  _Macro(4, yzxy, 1, 2, 0, 1) \
  _Macro(4, yzxz, 1, 2, 0, 2) \
  _Macro(4, yzyx, 1, 2, 1, 0) \
  _Macro(4, yzyy, 1, 2, 1, 1) \
  _Macro(4, yzyz, 1, 2, 1, 2) \
  _Macro(4, yzzx, 1, 2, 2, 0) \
  _Macro(4, yzzy, 1, 2, 2, 1) \
  _Macro(4, yzzz, 1, 2, 2, 2) \
  _Macro(4, zxxx, 2, 0, 0, 0) \
  _Macro(4, zxxy, 2, 0, 0, 1) \
  _Macro(4, zxxz, 2, 0, 0, 2) \
  _Macro(4, zxyx, 2, 0, 1, 0) \
  _Macro(4, zxyy, 2, 0, 1, 1) \
  _Macro(4, zxyz, 2, 0, 1, 2) \
  _Macro(4, zxzx, 2, 0, 2, 0) \
  _Macro(4, zxzy, 2, 0, 2, 1) \
  _Macro(4, zxzz, 2, 0, 2, 2) \
  _Macro(4, zyxx, 2, 1, 0, 0) \
  _Macro(4, zyxy, 2, 1, 0, 1) \
  _Macro(4, zyxz, 2, 1, 0, 2) \
  _Macro(4, zyyx, 2, 1, 1, 0) \
  _Macro(4, zyyy, 2, 1, 1, 1) \
  _Macro(4, zyyz, 2, 1, 1, 2) \
  _Macro(4, zyzx, 2, 1, 2, 0) \
  _Macro(4, zyzy, 2, 1, 2, 1) \
  _Macro(4, zyzz, 2, 1, 2, 2) \
  _Macro(4, zzxx, 2, 2, 0, 0) \
  _Macro(4, zzxy, 2, 2, 0, 1) \
  _Macro(4, zzxz, 2, 2, 0, 2) \
  _Macro(4, zzyx, 2, 2, 1, 0) \
  _Macro(4, zzyy, 2, 2, 1, 1) \
  _Macro(4, zzyz, 2, 2, 1, 2) \
  _Macro(4, zzzx, 2, 2, 2, 0) \
  _Macro(4, zzzy, 2, 2, 2, 1) \
  _Macro(4, zzzz, 2, 2, 2, 2)

#define FOREACH_CORE_SCALARVECTOR_SHUFFLE4(_Macro) \
  _Macro(2, xw, 0, 3) \
  _Macro(2, yw, 1, 3) \
  _Macro(2, zw, 2, 3) \
  _Macro(2, wx, 3, 0) \
  _Macro(2, wy, 3, 1) \
  _Macro(2, wz, 3, 2) \
  _Macro(2, ww, 3, 3) \
  _Macro(3, xxw, 0, 0, 3) \
  _Macro(3, xyw, 0, 1, 3) \
  _Macro(3, xzw, 0, 2, 3) \
  _Macro(3, xwx, 0, 3, 0) \
  _Macro(3, xwy, 0, 3, 1) \
  _Macro(3, xwz, 0, 3, 2) \
  _Macro(3, xww, 0, 3, 3) \
  _Macro(3, yxw, 1, 0, 3) \
  _Macro(3, yyw, 1, 1, 3) \
  _Macro(3, yzw, 1, 2, 3) \
  _Macro(3, ywx, 1, 3, 0) \
  _Macro(3, ywy, 1, 3, 1) \
  _Macro(3, ywz, 1, 3, 2) \
  _Macro(3, yww, 1, 3, 3) \
  _Macro(3, zxw, 2, 0, 3) \
  _Macro(3, zyw, 2, 1, 3) \
  _Macro(3, zzw, 2, 2, 3) \
  _Macro(3, zwx, 2, 3, 0) \
  _Macro(3, zwy, 2, 3, 1) \
  _Macro(3, zwz, 2, 3, 2) \
  _Macro(3, zww, 2, 3, 3) \
  _Macro(3, wxx, 3, 0, 0) \
  _Macro(3, wxy, 3, 0, 1) \
  _Macro(3, wxz, 3, 0, 2) \
  _Macro(3, wxw, 3, 0, 3) \
  _Macro(3, wyx, 3, 1, 0) \
  _Macro(3, wyy, 3, 1, 1) \
  _Macro(3, wyz, 3, 1, 2) \
  _Macro(3, wyw, 3, 1, 3) \
  _Macro(3, wzx, 3, 2, 0) \
  _Macro(3, wzy, 3, 2, 1) \
  _Macro(3, wzz, 3, 2, 2) \
  _Macro(3, wzw, 3, 2, 3) \
  _Macro(3, wwx, 3, 3, 0) \
  _Macro(3, wwy, 3, 3, 1) \
  _Macro(3, wwz, 3, 3, 2) \
  _Macro(3, www, 3, 3, 3) \
  _Macro(4, xxxw, 0, 0, 0, 3) \
  _Macro(4, xxyw, 0, 0, 1, 3) \
  _Macro(4, xxzw, 0, 0, 2, 3) \
  _Macro(4, xxwx, 0, 0, 3, 0) \
  _Macro(4, xxwy, 0, 0, 3, 1) \
  _Macro(4, xxwz, 0, 0, 3, 2) \
  _Macro(4, xxww, 0, 0, 3, 3) \
  _Macro(4, xyxw, 0, 1, 0, 3) \
  _Macro(4, xyyw, 0, 1, 1, 3) \
  _Macro(4, xyzw, 0, 1, 2, 3) \
  _Macro(4, xywx, 0, 1, 3, 0) \
  _Macro(4, xywy, 0, 1, 3, 1) \
  _Macro(4, xywz, 0, 1, 3, 2) \
  _Macro(4, xyww, 0, 1, 3, 3) \
  _Macro(4, xzxw, 0, 2, 0, 3) \
  _Macro(4, xzyw, 0, 2, 1, 3) \
  _Macro(4, xzzw, 0, 2, 2, 3) \
  _Macro(4, xzwx, 0, 2, 3, 0) \
  _Macro(4, xzwy, 0, 2, 3, 1) \
  _Macro(4, xzwz, 0, 2, 3, 2) \
  _Macro(4, xzww, 0, 2, 3, 3) \
  _Macro(4, xwxx, 0, 3, 0, 0) \
  _Macro(4, xwxy, 0, 3, 0, 1) \
  _Macro(4, xwxz, 0, 3, 0, 2) \
  _Macro(4, xwxw, 0, 3, 0, 3) \
  _Macro(4, xwyx, 0, 3, 1, 0) \
  _Macro(4, xwyy, 0, 3, 1, 1) \
  _Macro(4, xwyz, 0, 3, 1, 2) \
  _Macro(4, xwyw, 0, 3, 1, 3) \
  _Macro(4, xwzx, 0, 3, 2, 0) \
  _Macro(4, xwzy, 0, 3, 2, 1) \
  _Macro(4, xwzz, 0, 3, 2, 2) \
  _Macro(4, xwzw, 0, 3, 2, 3) \
  _Macro(4, xwwx, 0, 3, 3, 0) \
  _Macro(4, xwwy, 0, 3, 3, 1) \
  _Macro(4, xwwz, 0, 3, 3, 2) \
  _Macro(4, xwww, 0, 3, 3, 3) \
  _Macro(4, yxxw, 1, 0, 0, 3) \
  _Macro(4, yxyw, 1, 0, 1, 3) \
  _Macro(4, yxzw, 1, 0, 2, 3) \
  _Macro(4, yxwx, 1, 0, 3, 0) \
  _Macro(4, yxwy, 1, 0, 3, 1) \
  _Macro(4, yxwz, 1, 0, 3, 2) \
  _Macro(4, yxww, 1, 0, 3, 3) \
  _Macro(4, yyxw, 1, 1, 0, 3) \
  _Macro(4, yyyw, 1, 1, 1, 3) \
  _Macro(4, yyzw, 1, 1, 2, 3) \
  _Macro(4, yywx, 1, 1, 3, 0) \
  _Macro(4, yywy, 1, 1, 3, 1) \
  _Macro(4, yywz, 1, 1, 3, 2) \
  _Macro(4, yyww, 1, 1, 3, 3) \
  _Macro(4, yzxw, 1, 2, 0, 3) \
  _Macro(4, yzyw, 1, 2, 1, 3) \
  _Macro(4, yzzw, 1, 2, 2, 3) \
  _Macro(4, yzwx, 1, 2, 3, 0) \
  _Macro(4, yzwy, 1, 2, 3, 1) \
  _Macro(4, yzwz, 1, 2, 3, 2) \
  _Macro(4, yzww, 1, 2, 3, 3) \
  _Macro(4, ywxx, 1, 3, 0, 0) \
  _Macro(4, ywxy, 1, 3, 0, 1) \
  _Macro(4, ywxz, 1, 3, 0, 2) \
  _Macro(4, ywxw, 1, 3, 0, 3) \
  _Macro(4, ywyx, 1, 3, 1, 0) \
  _Macro(4, ywyy, 1, 3, 1, 1) \
  _Macro(4, ywyz, 1, 3, 1, 2) \
  _Macro(4, ywyw, 1, 3, 1, 3) \
  _Macro(4, ywzx, 1, 3, 2, 0) \
  _Macro(4, ywzy, 1, 3, 2, 1) \
  _Macro(4, ywzz, 1, 3, 2, 2) \
  _Macro(4, ywzw, 1, 3, 2, 3) \
  _Macro(4, ywwx, 1, 3, 3, 0) \
  _Macro(4, ywwy, 1, 3, 3, 1) \
  _Macro(4, ywwz, 1, 3, 3, 2) \
  _Macro(4, ywww, 1, 3, 3, 3) \
  _Macro(4, zxxw, 2, 0, 0, 3) \
  _Macro(4, zxyw, 2, 0, 1, 3) \
  _Macro(4, zxzw, 2, 0, 2, 3) \
  _Macro(4, zxwx, 2, 0, 3, 0) \
  _Macro(4, zxwy, 2, 0, 3, 1) \
  _Macro(4, zxwz, 2, 0, 3, 2) \
  _Macro(4, zxww, 2, 0, 3, 3) \
  _Macro(4, zyxw, 2, 1, 0, 3) \
  _Macro(4, zyyw, 2, 1, 1, 3) \
  _Macro(4, zyzw, 2, 1, 2, 3) \
  _Macro(4, zywx, 2, 1, 3, 0) \
  _Macro(4, zywy, 2, 1, 3, 1) \
  _Macro(4, zywz, 2, 1, 3, 2) \
  _Macro(4, zyww, 2, 1, 3, 3) \
  _Macro(4, zzxw, 2, 2, 0, 3) \
  _Macro(4, zzyw, 2, 2, 1, 3) \
  _Macro(4, zzzw, 2, 2, 2, 3) \
  _Macro(4, zzwx, 2, 2, 3, 0) \
  _Macro(4, zzwy, 2, 2, 3, 1) \
  _Macro(4, zzwz, 2, 2, 3, 2) \
  _Macro(4, zzww, 2, 2, 3, 3) \
  _Macro(4, zwxx, 2, 3, 0, 0) \
  _Macro(4, zwxy, 2, 3, 0, 1) \
  _Macro(4, zwxz, 2, 3, 0, 2) \
  _Macro(4, zwxw, 2, 3, 0, 3) \
  _Macro(4, zwyx, 2, 3, 1, 0) \
  _Macro(4, zwyy, 2, 3, 1, 1) \
  _Macro(4, zwyz, 2, 3, 1, 2) \
  _Macro(4, zwyw, 2, 3, 1, 3) \
  _Macro(4, zwzx, 2, 3, 2, 0) \
  _Macro(4, zwzy, 2, 3, 2, 1) \
  _Macro(4, zwzz, 2, 3, 2, 2) \
  _Macro(4, zwzw, 2, 3, 2, 3) \
  _Macro(4, zwwx, 2, 3, 3, 0) \
  _Macro(4, zwwy, 2, 3, 3, 1) \
  _Macro(4, zwwz, 2, 3, 3, 2) \
  _Macro(4, zwww, 2, 3, 3, 3) \
  _Macro(4, wxxx, 3, 0, 0, 0) \
  _Macro(4, wxxy, 3, 0, 0, 1) \
  _Macro(4, wxxz, 3, 0, 0, 2) \
  _Macro(4, wxxw, 3, 0, 0, 3) \
  _Macro(4, wxyx, 3, 0, 1, 0) \
  _Macro(4, wxyy, 3, 0, 1, 1) \
  _Macro(4, wxyz, 3, 0, 1, 2) \
  _Macro(4, wxyw, 3, 0, 1, 3) \
  _Macro(4, wxzx, 3, 0, 2, 0) \
  _Macro(4, wxzy, 3, 0, 2, 1) \
  _Macro(4, wxzz, 3, 0, 2, 2) \
  _Macro(4, wxzw, 3, 0, 2, 3) \
  _Macro(4, wxwx, 3, 0, 3, 0) \
  _Macro(4, wxwy, 3, 0, 3, 1) \
  _Macro(4, wxwz, 3, 0, 3, 2) \
  _Macro(4, wxww, 3, 0, 3, 3) \
  _Macro(4, wyxx, 3, 1, 0, 0) \
  _Macro(4, wyxy, 3, 1, 0, 1) \
  _Macro(4, wyxz, 3, 1, 0, 2) \
  _Macro(4, wyxw, 3, 1, 0, 3) \
  _Macro(4, wyyx, 3, 1, 1, 0) \
  _Macro(4, wyyy, 3, 1, 1, 1) \
  _Macro(4, wyyz, 3, 1, 1, 2) \
  _Macro(4, wyyw, 3, 1, 1, 3) \
  _Macro(4, wyzx, 3, 1, 2, 0) \
  _Macro(4, wyzy, 3, 1, 2, 1) \
  _Macro(4, wyzz, 3, 1, 2, 2) \
  _Macro(4, wyzw, 3, 1, 2, 3) \
  _Macro(4, wywx, 3, 1, 3, 0) \
  _Macro(4, wywy, 3, 1, 3, 1) \
  _Macro(4, wywz, 3, 1, 3, 2) \
  _Macro(4, wyww, 3, 1, 3, 3) \
  _Macro(4, wzxx, 3, 2, 0, 0) \
  _Macro(4, wzxy, 3, 2, 0, 1) \
  _Macro(4, wzxz, 3, 2, 0, 2) \
  _Macro(4, wzxw, 3, 2, 0, 3) \
  _Macro(4, wzyx, 3, 2, 1, 0) \
  _Macro(4, wzyy, 3, 2, 1, 1) \
  _Macro(4, wzyz, 3, 2, 1, 2) \
  _Macro(4, wzyw, 3, 2, 1, 3) \
  _Macro(4, wzzx, 3, 2, 2, 0) \
  _Macro(4, wzzy, 3, 2, 2, 1) \
  _Macro(4, wzzz, 3, 2, 2, 2) \
  _Macro(4, wzzw, 3, 2, 2, 3) \
  _Macro(4, wzwx, 3, 2, 3, 0) \
  _Macro(4, wzwy, 3, 2, 3, 1) \
  _Macro(4, wzwz, 3, 2, 3, 2) \
  _Macro(4, wzww, 3, 2, 3, 3) \
  _Macro(4, wwxx, 3, 3, 0, 0) \
  _Macro(4, wwxy, 3, 3, 0, 1) \
  _Macro(4, wwxz, 3, 3, 0, 2) \
  _Macro(4, wwxw, 3, 3, 0, 3) \
  _Macro(4, wwyx, 3, 3, 1, 0) \
  _Macro(4, wwyy, 3, 3, 1, 1) \
  _Macro(4, wwyz, 3, 3, 1, 2) \
  _Macro(4, wwyw, 3, 3, 1, 3) \
  _Macro(4, wwzx, 3, 3, 2, 0) \
  _Macro(4, wwzy, 3, 3, 2, 1) \
  _Macro(4, wwzz, 3, 3, 2, 2) \
  _Macro(4, wwzw, 3, 3, 2, 3) \
  _Macro(4, wwwx, 3, 3, 3, 0) \
  _Macro(4, wwwy, 3, 3, 3, 1) \
  _Macro(4, wwwz, 3, 3, 3, 2) \
  _Macro(4, wwww, 3, 3, 3, 3)
