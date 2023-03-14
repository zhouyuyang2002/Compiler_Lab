int x = 233;
const int y = 666;
int gcd(int x,int y){
  if (y == 0)
    return x;
  return gcd(y, x%y);
}
void f(){}
int main() {
  f();
  int z = getint();
  int ans = gcd(x, y + x + z);
  putch(ans);
  return 0;
}