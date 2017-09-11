cxx_library(
  name = 'rs-core',
  header_namespace = 'rs-core',
  exported_headers = subdir_glob([
    ('rs-core', '**/*.hpp'),
  ]),
  srcs = glob([
    'rs-core/**/*.cpp',
  ], excludes = glob([
    'rs-core/**/*-test.cpp',
  ])),
  visibility = [
    'PUBLIC',
  ],
)

cxx_binary(
  name = 'test',
  srcs = glob([
    'rs-core/**/*-test.cpp',
  ]),
  deps = [
    ':rs-core',
  ],
)
