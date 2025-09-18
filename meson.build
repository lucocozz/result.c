project('result.c', 'c',
  version : '0.1.0',
  default_options : ['c_std=c11', 'warning_level=3'])

inc = include_directories('.', is_system: true)

executable('result_example', 'examples/result.c',
  include_directories : inc)

executable('optional_example', 'examples/optional.c',
  include_directories : inc)

executable('optional_in_result_example', 'examples/optional_in_result.c',
  include_directories : inc)

executable('chaining_example', 'examples/chaining.c',
  include_directories : inc)

executable('map_example', 'examples/map.c',
  include_directories : inc)

executable('fail_fmt_example', 'examples/fail_fmt.c',
  include_directories : inc)
