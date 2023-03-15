int x = 233;
const int y = 666;
const int w = 114 * y;
int gcd(int x,int y){
  if (y == 0)
    return x;
  return gcd(y, x%y);
}
void f(){}
int main() {
  f();
  int z = getint();
  x = x + gcd(gcd(x * w + z, 2048), x);
  putch(x);
  return 0;
}