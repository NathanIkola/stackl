#include "stackl_debugger.h"

#include <iostream>
#include <cmath>
#include <cstdio>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdlib>
using std::runtime_error;
#include <set>
using std::set;

extern "C"
{
    #include "../vmem.h"
}

stackl_debugger::stackl_debugger( const char* filename )
{
    string fname = filename;
    _binary_name = fname;

    size_t idx =  fname.find_last_of( '.' );
    if( idx != string::npos )
        fname.erase( idx );

    try
    {
        _lst = asm_list( fname + ".dbg" );
        _ast = abstract_syntax_tree( fname + ".ast", _lst.max_ip() );
        load_commands();
        check_compile_time( filename );

        //if we get to this point it's because there were no exceptions thrown above
        _loaded = true;
        cout << "Debugger for " << _binary_name << " loaded. Type 'help' for help.\n";
    }
    catch( exception& ex )
    {
        _loaded = false;
        _failure_reason = ex.what();
    }
}

void stackl_debugger::check_compile_time( const char* filename )
{
    struct stat attrib;
    stat( filename, &attrib );
    if( attrib.st_mtime > _ast.compile_time() )
        throw runtime_error( "Debug files out of date." );
}

void stackl_debugger::load_commands()
{
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_breakpoint, { "breakpoint", "break", "b" }, "[line_num|file:line_num|func_name] - Adds breakpoint" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_removebreak, { "removebreak", "rbreak", "rb" }, "[line_num|file:line_num|func_name] - Removes breakpoint" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_print, { "print", "p" }, "[var_name|0xaddress] - Prints the value of the variable" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_continue, { "continue", "cont", "c" }, "- Runs program until the next breakpoint" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_list, { "list" }, " optional[line_count] - Prints all source code within N lines of the current line" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_next, { "next", "n" }, "[line_num|file:line_num|func_name] - Removes breakpoint" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_locals, { "locals", "listlocals" }, "- Print all local vars in current or specified function" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_globals, { "globals", "listglobals" }, "- Print all global variables" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_funcs, { "funcs", "functions", "listfuncs", "listfunctions" }, "- Print all functions" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_file, { "file", "currentfile" }, "- Prints the filename of the currently executing source code" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_program, { "program", "binary", "currentprogram", "currentbinary" }, "- Prints the filename of the loaded binary" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_func, { "func", "function", "currentfunc", "currentfunction" }, "- Prints the name of the currently executing function" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_exit, { "exit", "quit", "q" }, "- Exits current program" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_IP, { "IP" }, "- Prints the instruction pointer" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_FP, { "FP" }, "- Prints the frame pointer" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_BP, { "BP" }, "- Prints the base pointer" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_LP, { "LP" }, "- Prints the limit pointer" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_SP, { "SP" }, "- Prints the stack pointer" ) );
    _commands.push_back( debugger_command( *this, &stackl_debugger::cmd_help, { "help" }, "- Prints the help text" ) );
}

bool stackl_debugger::cmd_breakpoint( string& params, Machine_State* cpu )
{
    if( params.length() == 0 )
    {
        cout << "Specifiy a breakpoint location.\n";
        return true;
    }

    BREAKPOINT_RESULT res = add_breakpoint( params, cpu->IP );
    uint32_t final_addr;
    string file_name;

    switch( res )
    {
    case SUCCESS:
        final_addr = text_to_addr( params, cpu->IP );
        file_name = _lst.current_file( final_addr );
        cout << "breakpoint added at " << file_name << ':' << _lst.line_of_addr( file_name, final_addr ) << ".\n";
        break;
    case DUPLICATE:
        cout << "breakpoint already exists on that line.\n";
        break;
    case NOT_FOUND:
        cout << "couldn't find breakpoint location.\n";
        break;
    }
    return true;
}

bool stackl_debugger::cmd_removebreak( string& params, Machine_State* cpu )
{
    if( params.length() == 0 )
    {
        cout << "Specifiy a breakpoint location.\n";
        return true;
    }

    BREAKPOINT_RESULT res = remove_breakpoint( params, cpu->IP );
    uint32_t final_addr;
    string file_name;
    switch( res )
    {
        case SUCCESS:
            final_addr = text_to_addr( params, cpu->IP );
            file_name = _lst.current_file( final_addr );
            cout << "breakpoint removed from " << file_name << ':' << _lst.line_of_addr( file_name, final_addr ) << ".\n";
            break;
        case NOT_FOUND:
            cout << "breakpoint doesnt exist on that line.\n";
            break;
        case DUPLICATE:
            cout << "Not sure how you got here.\n";
            break;
    }
    return true;
}

bool stackl_debugger::cmd_print( string& params, Machine_State* cpu )
{
    if( params.length() == 0 )
    {
        cout << "Specifiy a variable to print.\n";
        return true;
    }

    try
    {
        cout << var_to_string( cpu, params ) << '\n';
    }
    catch( const char* err )
    {
        cout << err << '\n';
    }

    return true;
}

bool stackl_debugger::cmd_continue( string& params, Machine_State* cpu )
{
    return false;
}

bool stackl_debugger::cmd_list( string& params, Machine_State* cpu )
{
    int32_t range;
    if( params.length() == 0 )
        cout << get_nearby_lines( cpu->IP, 2 );

    else if( string_utils::is_number( params, 10, &range ) )
    cout << get_nearby_lines( cpu->IP, range );
    else
        cout << "[list] [optional range]\n";
    return true;
}

bool stackl_debugger::cmd_next( string& params, Machine_State* cpu )
{
    _prev_file = _lst.current_file( cpu->IP );
    _prev_line = _lst.line_of_addr( _prev_file, cpu->IP );
    _stepping = true;
    return false;
}

bool stackl_debugger::cmd_locals( string& params, Machine_State* cpu )
{
    if( params.length() == 0 )
    {
        string cur_func = _lst.current_func( cpu->IP );
        if( cur_func.empty() )
            cout << "Can't determine current function.\n";
        else
            cout << _ast.all_locals( cur_func );
    }
    else
    {
        if( _lst.addr_of_func( params ) != UINT32_MAX )
        {
            cout << _ast.all_locals( params );
        }
    }
    return true;
}

bool stackl_debugger::cmd_globals( string& params, Machine_State* cpu )
{
    cout << _ast.all_globals();
    return true;
}

bool stackl_debugger::cmd_funcs( string& params, Machine_State* cpu )
{
    cout << _ast.all_funcs();
    return true;
}

bool stackl_debugger::cmd_exit( string& params, Machine_State* cpu )
{
    exit( EXIT_SUCCESS );
    return true;
}

bool stackl_debugger::cmd_IP( string& params, Machine_State* cpu )
{
    cout << "Instruction Pointer: " << cpu->IP << '\n';
    return true;
}

bool stackl_debugger::cmd_FP( string& params, Machine_State* cpu )
{
    cout << "Frame Pointer: " << string_utils::to_hex( cpu->FP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_BP( string& params, Machine_State* cpu )
{
    cout << "Base Pointer: " << string_utils::to_hex( cpu->BP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_LP( string& params, Machine_State* cpu )
{
    cout << "Limit Pointer: " << string_utils::to_hex( cpu->LP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_SP( string& params, Machine_State* cpu )
{
    cout << "Stack Pointer: " << string_utils::to_hex( cpu->SP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_file( string& params, Machine_State* cpu )
{
    cout << _lst.current_file( cpu->IP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_program( string& params, Machine_State* cpu )
{
    cout << _binary_name << '\n';
    return true;
}

bool stackl_debugger::cmd_func( string& params, Machine_State* cpu )
{
    cout << _lst.current_func( cpu->IP ) << '\n';
    return true;
}

bool stackl_debugger::cmd_help( string& params, Machine_State* cpu )
{
    for( const debugger_command& cmd : _commands )
        cout << cmd.to_string() << '\n';
    return true;
}

stackl_debugger::BREAKPOINT_RESULT stackl_debugger::add_breakpoint( const string& break_point_text, uint32_t cur_addr )
{
    uint32_t bpl = text_to_addr( break_point_text, cur_addr );
    if( bpl != UINT32_MAX )
    {
	if( add_breakpoint( bpl ) )
	    return BREAKPOINT_RESULT::SUCCESS;
	else
	    return BREAKPOINT_RESULT::DUPLICATE;
    }
    else return BREAKPOINT_RESULT::NOT_FOUND;
}

stackl_debugger::BREAKPOINT_RESULT stackl_debugger::remove_breakpoint( const string & break_point_text, uint32_t cur_addr )
{
    uint32_t bpl = text_to_addr( break_point_text, cur_addr );
    if( bpl != UINT32_MAX )
    {
    	if( remove_breakpoint( bpl ) )
            return BREAKPOINT_RESULT::SUCCESS;
	else
            return BREAKPOINT_RESULT::NOT_FOUND;
    }
    else return BREAKPOINT_RESULT::NOT_FOUND;
}

uint32_t stackl_debugger::text_to_addr( const string& break_point_text, uint32_t cur_addr )
{
    int res;

    if( string_utils::begins_with( break_point_text, "0x" ) && string_utils::is_number( break_point_text, 16, &res ) )
    {
        return res;
    }
    else if( break_point_text.find( ':' ) != string::npos )
    {
        vector<string> file_line = string_utils::string_split( break_point_text, ':' );
	    return _lst.addr_of_line( file_line[0], stoi( file_line[1] ) );
    }
    else if( string_utils::is_number( break_point_text, 10, &res ) )
    {
        return _lst.addr_of_line( _lst.current_file( cur_addr ), res );
    }
    else
    {
	    return _lst.addr_of_func( break_point_text );
    }
}


bool stackl_debugger::add_breakpoint( uint32_t addr )
{
    if( find( _break_points.begin(), _break_points.end(), addr ) == _break_points.end() )
    {
    	_break_points.push_back( addr );
    	return true;
    }
    else return false;
}

bool stackl_debugger::remove_breakpoint( uint32_t addr )
{
    auto res = find( _break_points.begin(), _break_points.end(), addr );
    if( res != _break_points.end() )
    {
        _break_points.erase( res );
        return true;
    }
    else return false;
}

string stackl_debugger::var_to_string( Machine_State* cpu, const string& var_text )
{
    //array index does not work currently

    int32_t val;
    if( string_utils::begins_with( var_text, "0x" ) && string_utils::is_number( var_text, 16, &val ) )
        return string_utils::to_hex( Get_Word( cpu, val ) );

    string txt = var_text;

    //this function removes the end brackets if they exist
    //vector<uint32_t> idxs = string_utils::strip_array_indexes( txt );

    bool address_of = false;
    if( txt[0] == '&' )
    {
        address_of = true;
        txt.erase( 0, 1 );
    }

    uint32_t indirection = 0;
    while( txt[indirection++] == '*' ); //count the number of leading asterisks
    txt.erase( 0, --indirection ); //remove them

    vector<string> var_fields = string_utils::string_split( txt, '.' );

    vector<uint32_t> indexes = string_utils::strip_array_indexes( var_fields[0] );
    variable* var = _ast.var( _lst.current_func( cpu->IP ), var_fields[0] );

    if( var == nullptr )
        return "Variable not found in current scope";

    variable res = var->from_indexes( indexes );

    int32_t total_offset = res.offset();
    for( uint32_t i = 1; i < var_fields.size(); ++i )
    {
        if( res.is_struct() ) //the guy left of the '.' operator
        {
            indexes = string_utils::strip_array_indexes( var_fields[i] ); //strip ending brackets to get the 'true' var name
            var = res.decl()->var( var_fields[i] ); //ask the type of the left for the variable object of the guy on the right
            res = var->from_indexes( indexes ); //if there were any brackets, now we move through them
            total_offset += res.offset();
        }
        else
        {
            return string( "'" ) + var->definition() + "' is not a struct type.";
        }
    }

    //variable res = *var; //create copy of var
    res.offset( total_offset ); //modify its offset by the total dist from the FP
    res = res.deref( indirection, cpu );


    if( address_of )
        return string_utils::to_hex( res.total_offset( cpu ) );
    else return res.to_string( cpu );
}

void stackl_debugger::debug_check( Machine_State* cpu )
{
    if( !loaded() ) return;

    if( should_break( cpu->IP ) )
    {
        string cur_file = _lst.current_file( cpu->IP );
	    cout << "Breakpoint hit on " << cur_file << ":" << _lst.line_of_addr( cur_file, cpu->IP ) << '\n';
	    query_user( cpu );
    }
}

void stackl_debugger::query_user( Machine_State* cpu )
{
    string input;
    cout << "Enter a command.\n";
    while( true )
    {
        cout << "(dbg) ";
        getline( cin, input );

        if( input == "" )
        {
            input = _prev_cmd;
            cout << '\b' << _prev_cmd << '\n';
        }
        else
            _prev_cmd = input;

        //commands are always one word
        size_t idx = input.find_first_of( ' ' );
        string cmd, params;
        if( idx == string::npos )
        {
            cmd = input;
            params = "";
        }
        else
        {
            cmd = input.substr( 0, idx );
            params = input.substr( idx + 1 );
        }

        int32_t res = -1;
        //look at every command object and determine which one the user asked for
        for( const debugger_command& command : _commands )
        {
            if( command.called_by( cmd ) )
            { //the run the one they asked for
                res = command.run( params, cpu );
                break; //stop looking once we run our command
            }
        }
        if( res == -1 ) //if res wasn't modified, we never ran a command
            cout << "huh?\n";
        else if( res == 0 ) //return false means resume executing
            break;
        else continue; //otherwise prompt for input again
    }
}

bool stackl_debugger::should_break( uint32_t cur_addr )
{
    if( find( _break_points.begin(), _break_points.end(), cur_addr ) != _break_points.end() )
        return true;
    else if( _stepping )
    {
        string cur_file = _lst.current_file( cur_addr );
        uint32_t cur_line = _lst.line_of_addr( cur_file, cur_addr );
        if( cur_line == UINT32_MAX ) //if we step into a location undefined in the debug file
            return false; //then don't break
        if( cur_line != _prev_line || cur_file != _prev_file )
        { //if our current step has changed in either line or file, break.
            _stepping = false;
            return true;
        }
    }
    return false; //otherwise dont break
}

string stackl_debugger::get_nearby_lines( uint32_t cur_addr, int32_t range )
{
    string cur_file = _lst.current_file( cur_addr );
    int cur_line = (int)_lst.line_of_addr( cur_file, cur_addr );
    string ret = "";
    string line;
    ifstream file;

    try
    {
	    file.open( cur_file ); //this can throw an exception
	    if( !file.is_open() )
	        throw 0; //this is kinda dumb... but it keeps us from duplicating the error message?
    }
    catch( ... )
    {
	    return string( "Unable to open " ) + cur_file + '\n';
    }

    int i = 1; //files start on line 1
    while( getline( file, line ) )
    {
        if( i >= cur_line - range && i <= cur_line + range )
            ret += to_string( i ) + ". " + string( 3 - (int)log10( i ), ' ' ) + line + '\n';
        if( i > cur_line + range )
            break;
	    ++i;
    }
    return ret;
}