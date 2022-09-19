// Test that return types and existence matches function declaration
int foo()
{
    return;
}

void bar()
{
    return 1;
}

int main()
{
    foo();
    bar();
    return 0;
}
