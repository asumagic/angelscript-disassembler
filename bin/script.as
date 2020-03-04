void no_op() {}

/*

Disassembly:

```
Disassembly for void for_loop():
Variables:
- int i
Code:
@0x0000 SetV4 1 0
@0x0002 JMP 4
@0x0004 SUSPEND 
@0x0005 CALL 222
@0x0007 IncVi 1
@0x0008 CMPIi 1 10
@0x000a JS -8
@0x000c RET 0
```

Hand compiled from AS bytecode to textual LLVM IR:

```llvm
declare void @asllvm.suspend()

define void @asllvm.module.build.no_op() local_unnamed_addr #0 {
    ret void
}

define void @asllvm.module.build.for_loop() local_unnamed_addr #0 {
    ; @0x0000 SetV4 1 0
    %1 = alloca i32;
    store i32 0, i32* %1;

    ; @0x0002 JMP 4
    br label %reljmp1

reljmp2:
    ; @0x0004 SUSPEND
    ;call void() @asllvm.suspend();

    ; @0x0005 CALL 222
    call void() @asllvm.module.build.no_op()

    ; @0x0007 IncVi 1
    %2 = load i32, i32* %1
    %3 = add i32 %2, 1
    store i32 %3, i32* %1

    ; fallthrough
    br label %reljmp1

reljmp1:
    ; @0x0008 CMPIi 1 10
    ; @0x000a JS -8
    %4 = load i32, i32* %1
    %5 = icmp slt i32 %4, 10;
    br i1 %5, label %reljmp2, label %reljmp3

reljmp3:
    ret void
}
```

*/
void for_loop()
{
  for (int i = 0; i < 10; ++i)
  {
  	no_op();
  }
}

void simple_if(int a, int b)
{
	if (a > b)
	{
		no_op();
	}
}

int global_variable = 123;

int return_global()
{
	return global_variable;
}

void taking_one_parameter(int) {}

void taking_many_parameters(int a, int b, int c, int d)
{
	taking_one_parameter(a);
	taking_one_parameter(b);
	taking_one_parameter(c);
	taking_one_parameter(d);
}

void calling_many_parameters()
{
	taking_many_parameters(1, 2, 3, 4);
}

class C
{
	C() {}

	void foo() {}
}

void using_class()
{
	C c;
	c.foo();
}

void local_variables(int param_a, int param_b, int param_c)
{
	int a, b, c;

	a = 1;
	b = 2;
	c = 3;

	param_a = 1;
	param_b = 2;
	param_c = 3;
}

void different_size_locals()
{
	int8 a;
	int16 b;
	int32 c;
	int64 d;

	a = 1;
	b = 2;
	c = 3;
	d = 4;
}

void string_usage()
{
	string s;
	s += 123;
}

int add(int a, int b)
{
	return a + b;
}

int float_to_int_conversion(float x)
{
	return int(x);
}

C@ handle_manip()
{
	C@ c = C();
	return @c;
}

void handle_manip2()
{
	C@ c;
	C@ c2;

	@c = @c2;
}