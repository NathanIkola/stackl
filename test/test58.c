// Test switch statement semantics

int main()
{
    // test that duplicate cases fail to compile
    switch(1)
    {
        case 1:
        case 1:
        default:
            break;
    }

    // test that duplicate default cases fail to compile
    switch(1)
    {
        default:
        default:
            break;
    }
}