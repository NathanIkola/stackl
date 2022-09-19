// Test switch statements

#include <string.h>
#include <sysio.h>

int main()
{
    int i = 5;

    // test that it finds the correct case
    switch(i)
    {
        case 1:
            prints("1\n");
            break;
        case 2:
            prints("2\n");
            break;
        case 3:
            prints("3\n");
            break;
        case 4:
            prints("4\n");
            break;
        case 5:
            prints("5\n");
            break;
        case 6:
            prints("6\n");
            break;
        case 7:
            prints("7\n");
            break;
        case 8:
            prints("8\n");
            break;
        case 9:
            prints("9\n");
            break;
        case 10:
            prints("10\n");
            break;
    }

    // test that fallthrough behavior works correctly
    switch(i)
    {
        case 1:
            prints("1\n");
        case 2:
            prints("2\n");
        case 3:
            prints("3\n");
        case 4:
            prints("4\n");
        case 5:
            prints("5\n");
        case 6:
            prints("6\n");
        case 7:
            prints("7\n");
    }

    // test that unordered statements execute in the right order
    switch(i)
    {
        case 1:
            prints("1\n");
        case 3:
            prints("3\n");
        case 5:
            prints("5\n");
        case 2:
            prints("2\n");
        case 4:
            prints("4\n");
        case 6:
            prints("6\n");
    }

    // test that a nonexistent case hits the default
    switch(i)
    {
        case 1:
            prints("1\n");
        case 2:
            prints("2\n");
        case 3:
            prints("3\n");
        case 4:
            prints("4\n");
        case 6:
            prints("6\n");
        case 7:
            prints("7\n");
        default:
            prints("default\n");
    }

    // test that a nonexistent case without a default does nothing
    switch(i)
    {
        case 1:
            prints("1\n");
        case 2:
            prints("2\n");
        case 3:
            prints("3\n");
        case 4:
            prints("4\n");
        case 6:
            prints("6\n");
        case 7:
            prints("7\n");
    }

    // test that a break in a switch statement doesn't break out of enclosing loop
    char buf[256];
    int ii;
    for(ii = 0; ii < 5; ++ii)
    {
        prints(itostr(ii, buf));
        switch(ii)
        {
            case 3:
                prints("3");
                break;
        }
    }

    // test that a continue inside of a switch continues
    for(ii = 0; ii < 5; ++ii)
    {
        switch(ii)
        {
            case 3:
                continue;
                prints("Failed to continue from within switch");
        }
        prints(itostr(ii, buf));
    }

    return 0;
}