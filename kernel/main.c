/*
ZAVATTARO CAMILLO
26/04/2021
Primary LeoKernel module
*/

/*
to do:
- check A20
- configure pci devices
- gather hardware informations
- load idt
- enable paging
*/

int a;
int d;
int b = 0;
int *c = &b;

int foo() {
  return 9;
}

int kmain() {
  a = 1;
  b += a;
  d = foo();
  return a;
}