#ifndef LANGUAGES_H
#define LANGUAGES_H

#include "highlighting.h"
#include <vector>
#include "la.h"
const std::vector<Language> LANGUAGES = {
{
  "C/C++",
  {"for", "while", "if", "int", "float", "return", "double", "true", "false", "else", "void", "delete", "struct", "class", "case", "break", "const", "nullptr", "auto", "bool", "new", "auto*", "switch", "case", "typedef", "static", "enum"},
  {"#include", "std::", "#ifdef", "#define", "#endif", "#ifndef"},
  "//",
  {"/*", "*/"},
  "\"'",
  '\\',
  {"cc", "h", "cpp", "hpp", "c"}
},
{
  "JavaScript",
  {"async", "null", "undefined", "as", "from", "abstract","arguments","await","boolean","break","byte","case","catch","char","class","const","continue","debugger","default", "let","delete","do","double","else","enum","eval","export","extends","false","final","finally","float","for","function","goto","if","implements","import","in","instanceof","int","interfacelet","long","native","new","null","package","private","protected","public","return","short","static","super","switch","synchronized","this","throw","throws","transient","true","try","typeof","var","void","volatile","while","with","yield"},
  {"process", "requestAnimationFrame", "window", "console", "Object", "Array", "Number", "String", "JSON", "Buffer", "require"},
  "//",
  {"/*", "*/"},
  "\"'`",
  '\\',
  {"js", "jsx"}
},
{
  "Python",
  { "and", "as", "assert", "break", "class", "continue", "def", "del", "elif", "else", "except", "False", "finally", "for", "from", "global", "if", "import", "in", "is", "lambda", "None", "nonlocal", "not", "or", "pass", "raise", "return", "True", "try", "while", "with", "yield"},
  {"print", "dict", "range", "open"},
  "#",
  {"\"\"\"", "\"\"\""},
  "\"'",
  '\\',
  {"py"}
},
{
  "Go",
  {"break", "default", "func", "interface", "select", "case", "defer", "go", "map", "struct", "chan", "else", "goto", "package", "switch", "const", "fallthrough", "if", "range", "type", "continue", "for", "import", "return", "var"},
  {"append","bool","byte","cap","close","complex","complex64","complex128","uint16","copy","false","float32","float64","imag","int","int8","int16","uint32","int32","int64","iota","len","make","new","nil","panic","uint64","print","println","real","recover","string","true","uint","uint8","uintptr"},
  "//",
  {"/*","*/"},
  "\"'",
  '\\',
  {"go"}
},
{
  "TypeScript",
  {"async", "await", "break","case","catch","class","const","continue","debugger","default","delete","do","else","enum","export","extends","false","finally","for","function","if","import","in","instanceof","new","null","return","super","switch","this","throw","true","try","typeof","var","void","while","with","as","implements","interface","let","package","private","protected","public","static","yield", "any","boolean","constructor","declare","get","module","require","number","set","string","symbol","type","from","of"},
  {"process", "requestAnimationFrame", "window", "console", "Object", "Array", "Number", "String", "JSON", "Buffer", "require"},
  "//",
  {"/*", "*/"},
  "\"'`",
  '\\',
  {"ts", "tsx"}
},
{
  "Dockerfile",
  { "CMD", "ENTRYPOINT", "RUN", "ADD", "COPY", "ENV", "EXPOSE", "FROM", "LABEL", "STOPSIGNAL", "USER", "VOLUME", "WORKDIR", "ONBUILD", "AS" },
  {},
  "#",  
  {"",""},
  "\"",
  '\\',
  {"dockerfile"}
},
{
 "Elixir",
 {"def","defp","defn","defnp","defguard","defguardp","defmacro","defmacrop","defdelegate","defcallback","defmacrocallback","defmodule","defprotocol","defexception","defimpl","defstruct","and", "in", "not", "or", "when", "alias", "import", "require", "use","after","case","catch","cond","do","else","end","fn","for","if","quote","raise","receive","rescue","super","throw","try","unless","unquote_splicing","unquote", "true", "false", "nil", "with"  },
 {"__MODULE__", "__DIR__", "__ENV__", "__CALLER__", "__STACKTRACE__"},
 "#",
 {"",""},
 "\"'",
 '\\',
 {"ex", "exs"}
},
{
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
},
{
  "JSON",
  {"true", "false", "null"},
  {},
  "",
  {"",""},
  "\"",
  '\\',
  {"json"}
}
};
const Language* has_language(std::string ext) {
  for (const auto& lang : LANGUAGES) {
  if(std::find(lang.fileExtensions.begin(), lang.fileExtensions.end(), ext) != lang.fileExtensions.end())
    return &lang;
  }
  return nullptr;
}

#endif
