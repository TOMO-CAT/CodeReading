prebuilt_cc_library(
    name='gmock',
    export_incs=['include'],
    libpath_pattern='lib',
    hdrs=[
        'include/gmock/gmock.h',
    ],
    deps=[
        '#pthread',
    ],
    visibility=['PUBLIC'],
)

prebuilt_cc_library(
    name='gmock_main',
    export_incs=['include'],
    libpath_pattern='lib',
    hdrs=[
        'include/gmock/gmock.h'
    ],
    visibility=['PUBLIC'],
)

cc_library(
    name='gmock_lib',
    deps=[
        ':gmock',
        ':gmock_main',
    ],
    # warning='no',
    hdrs=[],
    visibility=['PUBLIC'],
)
