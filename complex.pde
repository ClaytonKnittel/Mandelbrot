class Complex {
  
  double r, i;
  
  Complex(double real, double imag) {
    r = real;
    i = imag;
  }
  
  Complex add(Complex c) {
    return new Complex(r + c.r, i + c.i);
  }
  
  Complex mul(Complex c) {
    return new Complex(r * c.r - i * c.i, r * c.i + i * c.r);
  }
  
  Complex square() {
    return this.mul(this);
  }
  
  double mag() {
    return Math.sqrt(r * r + i * i);
  }
  
  public String toString() {
    return r + (i < 0 ? " - " : " + ") + abs((float) i);
  }
  
}
