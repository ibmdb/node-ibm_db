def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  if not conf.check(lib="odbc", libpath=['/usr/local/lib', '/opt/local/lib'], uselib_store="ODBC"):
    conf.fatal('Missing libodbc');
  conf.env.append_value('LIBPATH_ODBC', '/opt/local/lib');

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = ["-g", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]
  obj.target = "odbc_bindings"
  obj.source = "src/Database.cpp"
  obj.uselib = "ODBC"