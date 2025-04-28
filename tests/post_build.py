import regis.run_clang_tools
import regis.diagnostics
import argparse
import os
import sys

def run(clangToolsConfig):
  if (clangToolsConfig):
    regis.run_clang_tools.run_from_config(clangToolsConfig)

  return

if __name__ == "__main__":
  parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)

  parser.add_argument("-p", "--project", help="project name")
  parser.add_argument("-comp", "--compiler", help="compiler")
  parser.add_argument("-conf", "--config", help="configuration")
  parser.add_argument("-l", "--level", default="info", help="logging level")
  parser.add_argument("-srcroot", help="src root folder")
  parser.add_argument("-clang-tools-config", help="path to the clang tools config.", default='')
  args, unknown = parser.parse_known_args()

  project_name = args.project
  compiler = args.compiler
  config = args.config
  srcroot = args.srcroot
  clang_tools_config_path = args.clang_tools_config
 # initialize the logger
  log_level_str = args.level
  log_level = regis.diagnostics.logging_level_from_string(log_level_str)
  logger = regis.diagnostics.StreamLogger("post_build", log_level)

 # useful for debugging
  logger.info(f"Executing {__file__}")

 # execute the script
  run(clang_tools_config_path)

 # user hook
  project_post_build_script = os.path.join(srcroot, 'post_build.py')
  if os.path.isfile(project_post_build_script):
    arguments = ' '.join(sys.argv[1:]) # skip the first arguments as that's the path to this script
    logger.info(f'running project post build "{project_post_build_script}"')
    logger.info(f'with args: {arguments}')
    os.system(f'py {project_post_build_script} {arguments}')
  else:
    logger.debug('not running user defined post build events')
    logger.debug('as there is no "post_build.py" found at source root.')
    logger.debug(f'source root: {srcroot}')

 # print. We're done.
  logger.info("Done.")

  exit(0)
