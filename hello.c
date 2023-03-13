int main() {
  int a = 1;
  int b = 0;
  int c = 5;
  if (a || b)
    if (a && b)
      c = c * 233;
    else
      return c * 666;
  return 19260817 / c;
}
