#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "highlighting.h"
#include "la.h"
const Language CPP = {
  "C/C++",
  {"for", "while", "if", "int", "float", "return", "double", "true", "false", "else", "void", "delete", "struct", "class", "case", "break", "const", "nullptr", "auto", "bool", "new", "auto*", "switch", "case", "typedef", "static", "enum"},
  {"#include", "std::", "#ifdef", "#define", "#endif", "#ifndef"},
  "//",
  {"/*", "*/"},
  "\"'",
  '\\',
  {"cc", "h", "cpp", "hpp", "c"}
};
const Language JavaScript = {
  "JavaScript",
  {"null", "undefined", "as", "from", "abstract","arguments","await","boolean","break","byte","case","catch","char","class","const","continue","debugger","default", "let","delete","do","double","else","enum","eval","export","extends","false","final","finally","float","for","function","goto","if","implements","import","in","instanceof","int","interfacelet","long","native","new","null","package","private","protected","public","return","short","static","super","switch","synchronized","this","throw","throws","transient","true","try","typeof","var","void","volatile","while","with","yield"},
  {"process", "requestAnimationFrame", "window", "console", "Object", "Array", "Number", "String", "JSON", "Buffer", "require"},
  "//",
  {"/*", "*/"},
  "\"'`",
  '\\',
  {"js"}
};
const Language Python = {
  "Python",
  { "and", "as", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "False", "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "None", "nonlocal", "not", "or", "pass", "raise", "return", "True", "try", "while", "with", "yield"},
  {"print", "dict", "range", "open"},
  "#",
  {"\"\"\"", "\"\"\""},
  "\"'",
  '\\',
  {"py"}
};
const Language Go = {
  "Go",
  {"break", "default", "func", "interface", "select", "case", "defer", "go", "map", "struct", "chan", "else", "goto", "package", "switch", "const", "fallthrough", "if", "range", "type", "continue", "for", "import", "return", "var"},
  {"append","bool","byte","cap","close","complex","complex64","complex128","uint16","copy","false","float32","float64","imag","int","int8","int16","uint32","int32","int64","iota","len","make","new","nil","panic","uint64","print","println","real","recover","string","true","uint","uint8","uintptr"},
  "//",
  {"/*","*/"},
  "\"'",
  '\\',
  {"go"}
};

const Language TypeScript {
  "TypeScript",
  {"break","case","catch","class","const","continue","debugger","default","delete","do","else","enum","export","extends","false","finally","for","function","if","import","in","instanceof","new","null","return","super","switch","this","throw","true","try","typeof","var","void","while","with","as","implements","interface","let","package","private","protected","public","static","yield", "any","boolean","constructor","declare","get","module","require","number","set","string","symbol","type","from","of"},
  {"process", "requestAnimationFrame", "window", "console", "Object", "Array", "Number", "String", "JSON", "Buffer", "require"},
  "//",
  {"/*", "*/"},
  "\"'`",
  '\\',
  {"ts"}
};
const Language Dockerfile = {
  "Dockerfile",
  { "CMD", "ENTRYPOINT", "RUN", "ADD" "COPY", "ENV", "EXPOSE", "FROM", "LABEL", "STOPSIGNAL", "USER", "VOLUME", "WORKDIR", "ONBUILD", "AS" },
  {},
  "#",  
  {"",""},
  "\"",
  '\\',
  {"dockerfile"}
};
const Language Elixir = {
 "Elixir",
 {"def","defp","defn","defnp","defguard","defguardp","defmacro","defmacrop","defdelegate","defcallback","defmacrocallback","defmodule","defprotocol","defexception","defimpl","defstruct","and", "in", "not", "or", "when", "alias", "import", "require", "use","after","case","catch","cond","do","else","end","fn","for","if","quote","raise","receive","rescue","super","throw","try","unless","unquote_splicing","unquote", "true", "false", "nil", "with"  },
 {"__MODULE__", "__DIR__", "__ENV__", "__CALLER__", "__STACKTRACE__"},
 "#",
 {"",""},
 "\"'",
 '\\',
 {"ex", "exs"}
};
const Language Shell = {
  "Shell",
  {
"if",
"then",
"do",
"else",
"elif",
"while",
"until",
"for",
"in",
"esac",
"fi",
"fin",
"fil",
"done",
"exit",
"set",
"unset",
"export",
"function"
},
{
"ab",
"awk",
"bash",
"beep",
"cat",
"cc",
"cd",
"chown",
"chmod",
"chroot",
"clear",
"cp",
"curl",
"cut",
"diff",
"echo",
"find",
"gawk",
"gcc",
"get",
"git",
"grep",
"hg",
"kill",
"killall",
"ln",
"ls",
"make",
"mkdir",
"openssl",
"mv",
"nc",
"node",
"npm",
"ping",
"ps",
"restart",
"rm",
"rmdir",
"sed",
"service",
"sh",
"shopt",
"shred",
"source",
"sort",
"sleep",
"ssh",
"start",
"stop",
"su",
"sudo",
"svn",
"tee",
"telnet",
"top",
"touch",
"vi",
"vim",
"wall",
"wc",
"wget",
"who",
"write",
"yes",
"zsh"
},
 "#",
  {"",""},
  "\"'",
 '\\',
 {"sh", "bash"}
};
const Language* has_language(std::string ext) {
  if(std::find(CPP.fileExtensions.begin(), CPP.fileExtensions.end(), ext)!=  CPP.fileExtensions.end())
    return &CPP;
  if(std::find(JavaScript.fileExtensions.begin(), JavaScript.fileExtensions.end(), ext) != JavaScript.fileExtensions.end())
    return &JavaScript;
  if(std::find(Python.fileExtensions.begin(), Python.fileExtensions.end(), ext) != Python.fileExtensions.end())
    return &Python;
  if(std::find(Go.fileExtensions.begin(), Go.fileExtensions.end(), ext) != Go.fileExtensions.end())
    return &Go;
  if(std::find(TypeScript.fileExtensions.begin(), TypeScript.fileExtensions.end(), ext) != TypeScript.fileExtensions.end())
    return &TypeScript;
  if(std::find(Dockerfile.fileExtensions.begin(), Dockerfile.fileExtensions.end(), ext) != Dockerfile.fileExtensions.end())
    return &Dockerfile;
  if(std::find(Elixir.fileExtensions.begin(), Elixir.fileExtensions.end(), ext) != Elixir.fileExtensions.end())
    return &Elixir;
  if(std::find(Shell.fileExtensions.begin(), Shell.fileExtensions.end(), ext) != Shell.fileExtensions.end())
    return &Shell;
  return nullptr;
}
#endif
