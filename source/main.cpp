#include <iostream>  // cout
#include <iomanip>
#include <assert.h>  // assert()
#include <string.h>  // strstr()
#include <angelscript.h>
#include "../../../add_on/scriptbuilder/scriptbuilder.h"
#include "../../../add_on/scripthelper/scripthelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#if defined(_MSC_VER) && !defined(_WIN32_WCE)
#include <direct.h>
#include <crtdbg.h>
#endif
#ifdef _WIN32_WCE
#include <windows.h> // For GetModuleFileName
#endif

using namespace std;

// Function prototypes
int ConfigureEngine(asIScriptEngine *engine, const char *configFile);
int CompileScript(asIScriptEngine *engine, const char *scriptFile);
int SaveBytecode(asIScriptEngine *engine, const char *outputFile);
static const char *GetCurrentDir(char *buf, size_t size);

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING ) 
		type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION ) 
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

std::pair<const asDWORD*, const asDWORD*> get_byte_code(asIScriptFunction* function)
{
	asUINT length;
	const asDWORD* begin = function->GetByteCode(&length);
	const asDWORD* end = begin + length;

	return {begin, end};
}

void display_operands(const asDWORD* current, asEBCType type)
{
	switch (type)
	{
	case asBCTYPE_NO_ARG:
	{
		break;
	}

	case asBCTYPE_W_ARG:
	case asBCTYPE_wW_ARG:
	case asBCTYPE_rW_ARG:
	{
		cout << asBC_SWORDARG0(current);
		break;
	}

	case asBCTYPE_DW_ARG:
	{
		cout << asBC_INTARG(current);
		break;
	}

	case asBCTYPE_rW_DW_ARG:
	case asBCTYPE_wW_DW_ARG:
	case asBCTYPE_W_DW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_INTARG(current);
		break;
	}

	case asBCTYPE_QW_ARG:
	{
		cout << asBC_PTRARG(current);
		break;
	}

	case asBCTYPE_DW_DW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_INTARG(current + 1);
		break;
	}

	case asBCTYPE_wW_rW_rW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_SWORDARG1(current) << ' ' << asBC_SWORDARG0(current + 1);
		break;
	}

	case asBCTYPE_wW_QW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_PTRARG(current + 1);
		break;
	}

	case asBCTYPE_wW_rW_ARG:
	case asBCTYPE_rW_rW_ARG:
	case asBCTYPE_wW_W_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_SWORDARG1(current);
		break;
	}

	case asBCTYPE_wW_rW_DW_ARG:
	case asBCTYPE_rW_W_DW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_SWORDARG1(current) << ' ' << asBC_INTARG(current + 1);
		break;
	}

	case asBCTYPE_QW_DW_ARG:
	{
		cout << asBC_PTRARG(current) << ' ' << asBC_INTARG(current + 2);
		break;
	}

	case asBCTYPE_rW_QW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_PTRARG(current + 1);
		break;
	}

	case asBCTYPE_rW_DW_DW_ARG:
	{
		cout << asBC_SWORDARG0(current) << ' ' << asBC_INTARG(current + 1) << ' ' << asBC_INTARG(current + 2);
		break;
	}

	default:
	{
		cout << "(operands unimplemented for this type)";
		break;
	}
	}
}

void disassemble(asIScriptFunction* function)
{
	cout << "\nDisassembly for " << function->GetDeclaration(true, true, true) << ":\n";

	cout << "Variables:\n";

	for (int i = 0, var_count = function->GetVarCount(); i < var_count; ++i)
	{
		cout << "- " << function->GetVarDecl(i, true) << '\n';
	}

	cout << "Code:\n";

	const auto [begin, end] = get_byte_code(function);
	const asDWORD* current = begin;

	if (current == nullptr)
	{
		cout << "Disassembly failed, byte code could not be read\n";
		return;
	}

	while (current < end)
	{
		const asSBCInfo info = asBCInfo[*reinterpret_cast<const asBYTE*>(current)];
		const std::size_t instruction_size = asBCTypeSize[info.type];

		cout << "@0x" << std::hex << std::setw(4) << std::setfill('0') << std::distance(begin, current) << ' '
			 << info.name << ' ';


		cout << std::dec;

		display_operands(current, info.type);

		cout << '\n';

		current += instruction_size;
	}
}

void disassemble(asIScriptEngine* engine)
{
	cout << "Now disassembling bytecode.\n";

	const asIScriptModule* module = engine->GetModule("build");
	for (int i = 0, function_count = module->GetFunctionCount(); i < function_count; ++i)
	{
		disassemble(module->GetFunctionByIndex(i));
	}

	for (int i = 0, type_count = module->GetObjectTypeCount(); i < type_count; ++i)
	{
		asITypeInfo* info = module->GetObjectTypeByIndex(i);

		for (int j = 0, method_count = info->GetBehaviourCount(); j < method_count; ++j)
		{
			disassemble(info->GetBehaviourByIndex(j, nullptr));
		}

		for (int j = 0, method_count = info->GetMethodCount(); j < method_count; ++j)
		{
			disassemble(info->GetMethodByIndex(j));
		}
	}
}

int main(int argc, char **argv)
{
	int r;

	if( argc < 4 )
	{
		cout << "Usage: " << endl;
		cout << "asbuild <config file> <script file> <output>" << endl;
		cout << " <config file>  is the file with the application interface" << endl;
		cout << " <script file>  is the script file that should be compiled" << endl;
		cout << " <output>       is the name that the compiled script will be saved as" << endl;
		return -1;
	}

	// Create the script engine
	asIScriptEngine *engine = asCreateScriptEngine();
	if( engine == 0 )
	{
		cout << "Failed to create script engine." << endl;
		return -1;
	}

	// The script compiler will send any compiler messages to the callback
	engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

	// Configure the script engine with all the functions, 
	// and variables that the script should be able to use.
	r = ConfigureEngine(engine, argv[1]);
	if( r < 0 ) return -1;
	
	// Compile the script code
	r = CompileScript(engine, argv[2]);
	if( r < 0 ) return -1;

	disassemble(engine);

	// Shut down the engine
	engine->ShutDownAndRelease();

	return 0;
}

#ifdef AS_CAN_USE_CPP11
// The string factory doesn't need to keep a specific order in the
// cache, so the unordered_map is faster than the ordinary map
#include <unordered_map>  // std::unordered_map
BEGIN_AS_NAMESPACE
typedef unordered_map<string, int> map_t;
END_AS_NAMESPACE
#else
#include <map>      // std::map
BEGIN_AS_NAMESPACE
typedef map<string, int> map_t;
END_AS_NAMESPACE
#endif

// Default string factory. Removes duplicate string constants
// This same implementation is provided in the scriptstdstring add-on
class CStdStringFactory : public asIStringFactory
{
public:
	CStdStringFactory() {}
	~CStdStringFactory()
	{
		// The script engine must release each string
		// constant that it has requested
		assert(stringCache.size() == 0);
	}

	const void *GetStringConstant(const char *data, asUINT length)
	{
		string str(data, length);
		map_t::iterator it = stringCache.find(str);
		if (it != stringCache.end())
			it->second++;
		else
			it = stringCache.insert(map_t::value_type(str, 1)).first;

		return reinterpret_cast<const void*>(&it->first);
	}

	int  ReleaseStringConstant(const void *str)
	{
		if (str == 0)
			return asERROR;

		map_t::iterator it = stringCache.find(*reinterpret_cast<const string*>(str));
		if (it == stringCache.end())
			return asERROR;

		it->second--;
		if (it->second == 0)
			stringCache.erase(it);
		return asSUCCESS;
	}

	int  GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (str == 0)
			return asERROR;

		if (length)
			*length = (asUINT)reinterpret_cast<const string*>(str)->length();

		if (data)
			memcpy(data, reinterpret_cast<const string*>(str)->c_str(), reinterpret_cast<const string*>(str)->length());

		return asSUCCESS;
	}

	// TODO: Make sure the access to the string cache is thread safe
	map_t stringCache;
};

CStdStringFactory stringFactory;

// This function will register the application interface, 
// based on information read from a configuration file. 
int ConfigureEngine(asIScriptEngine *engine, const char *configFile)
{
	int r;

	ifstream strm;
	strm.open(configFile);
	if( strm.fail() )
	{
		// Write a message to the engine's message callback
		char buf[256];
		string msg = "Failed to open config file";
		engine->WriteMessage(configFile, 0, 0, asMSGTYPE_ERROR, msg.c_str());
		return -1;
	}

	// Configure the engine with the information from the file
	r = ConfigEngineFromStream(engine, strm, configFile, &stringFactory);
	if( r < 0 )
	{
		engine->WriteMessage(configFile, 0, 0, asMSGTYPE_ERROR, "Configuration failed");
		return -1;
	}

	engine->WriteMessage(configFile, 0, 0, asMSGTYPE_INFORMATION, "Disabling line clues and introducing JIT entries");
	//engine->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, true);
	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);

	engine->WriteMessage(configFile, 0, 0, asMSGTYPE_INFORMATION, "Configuration successfully registered");
	
	return 0;
}

int CompileScript(asIScriptEngine *engine, const char *scriptFile)
{
	int r;

	CScriptBuilder builder;
	r = builder.StartNewModule(engine, "build");
	if( r < 0 ) return -1;

	r = builder.AddSectionFromFile(scriptFile);
	if( r < 0 ) return -1;

	r = builder.BuildModule();
	if( r < 0 )
	{
		engine->WriteMessage(scriptFile, 0, 0, asMSGTYPE_ERROR, "Script failed to build");
		return -1;
	}

	engine->WriteMessage(scriptFile, 0, 0, asMSGTYPE_INFORMATION, "Script successfully built");

	return 0;
}

