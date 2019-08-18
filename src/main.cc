#include <iostream>
#include <string>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"

struct Foo {
  void operator()(int x) {
    n += x;
    std::cout << "Foo::()(" << x << ") = " << n << std::endl;
  }
  int n = 0;
};

struct Bar {
  void add(int x) {
    n += x;
    std::cout << "Bar::add(" << x << "), sum = " << n << std::endl;
  }
  int n = 0;
};

void test(int x) {
  std::cout << x << std::endl;
}

void test_unique(std::unique_ptr<int> x) {
  std::cout << "x(" << x.get() << ") = " << *x << std::endl;
}

int add2(int x, int y) {
  return x + y;
}

void complex_func(const char* a1,
                  int a2,
                  short a3,
                  std::string a4,
                  double a5,
                  long a6) {
  std::cout << a1 << " " << a2 << " " << a3 << " " << a4 << " " << a5 << " "
            << a6 << std::endl;
}

struct complex_class {
  void complex_member_func(const char* a1, std::string a2, float a3) {
    std::cout << a1 << " " << a2 << " " << a3 << std::endl;
  }
};

void test_callback() {
  base::RepeatingCallback<void(int)> c(&test);
  c.Run(7);
  base::RepeatingCallback<void(double)> d(
      [](double x) { std::cout << "double: " << x << std::endl; });
  d.Run(2.71);

  Bar bar;
  base::RepeatingCallback<void(int)> h(&Bar::add, &bar);
  h.Run(41);
  h.Run(1);

  base::RepeatingCallback<void()> bc2{&test, std::make_tuple(842)};
  bc2.Run();
  base::RepeatingCallback<void()> bc4{&Bar::add, &bar, std::make_tuple(521)};
  bc4.Run();

  base::RepeatingCallback<int(int, int)> bc5{&add2};
  std::cout << "3 + 7 = " << bc5.Run(3, 7) << std::endl;

  base::RepeatingCallback<int(int)> bc6{&add2, std::make_tuple(1)};
  std::cout << "increment(5) = " << bc6.Run(5) << std::endl;
}

void test_bind() {
  auto b1 = base::BindRepeating(&test);
  b1.Run(11);
  b1.Run(12);

  auto b2 = base::BindRepeating(&test, 20);
  b2.Run();
  b2.Run();

  Bar bar;
  auto b3 = base::BindRepeating(&Bar::add, &bar);
  b3.Run(31);
  b3.Run(32);

  auto b4 = base::BindRepeating(&Bar::add, &bar, 40);
  b4.Run();
  b4.Run();

  auto b5 = base::BindRepeating(&add2);
  std::cout << "4 + 3 = " << b5.Run(4, 3) << std::endl;

  auto b6 = base::BindRepeating(&add2, 1);
  std::cout << "increment(2) = " << b6.Run(2) << std::endl;
}

void test_advanced_bind() {
  auto b1 = base::BindRepeating(&complex_func);
  b1.Run("test #1.1:", 10, 5, "abc", 3.14, 100);
  b1.Run("test #1.2:", 4521, 224, std::string("foo"), 2.71, 425151);

  auto b2 = base::BindRepeating(b1, "test #2.x:", 37);
  b2.Run(3, "bar", 1.17, 9321);
  b2.Run(-2, std::string("baz"), -99.1, -1);

  auto b3 = base::BindRepeating(b2, 37, "[bounded]");
  b3.Run(-12.52, 42);
  b3.Run(8.152, 4242);

  complex_class cs;
  auto b4 = base::BindRepeating(&complex_class::complex_member_func, &cs);
  b4.Run("mem_test #1", "foo_bar", 1.123f);

  auto b5 = base::BindRepeating(b4, "mem_test [bounded]");
  b5.Run("baz_foo", 3.321f);

  auto b6 = base::BindRepeating(b5, "bounded-string", 2.71111f);
  b6.Run();
  b6.Run();
}

void test_once() {
  auto b1 = base::OnceCallback<void(std::unique_ptr<int>)>(&test_unique);
  std::move(b1).Run(std::make_unique<int>(1));

  auto b2 = base::OnceCallback<void()>(
      &test_unique, std::make_tuple(std::make_unique<int>(2)));
  std::move(b2).Run();

  auto b3_helper = base::OnceCallback<void(std::unique_ptr<int>)>(&test_unique);
  auto b3 = base::OnceCallback<void()>(
      std::move(b3_helper), std::make_tuple(std::make_unique<int>(3)));
  std::move(b3).Run();

  auto b4 = base::BindOnce(&test_unique);
  std::move(b4).Run(std::make_unique<int>(4));

  auto b5 = base::BindOnce(&test_unique, std::make_unique<int>(5));
  std::move(b5).Run();

  auto b6_helper = base::BindOnce(&test_unique);
  auto b6 = base::BindOnce(std::move(b6_helper), std::make_unique<int>(6));
  std::move(b6).Run();

  base::RepeatingCallback<void(int)> b7_repeating = base::BindRepeating(&test);
  base::OnceCallback<void(int)> b7_once = b7_repeating;
  std::move(b7_once).Run(7);
}

//
//
//

int main() {
  test_callback();
  test_bind();
  test_advanced_bind();
  test_once();

  return 0;
}
