{
  'targets': [
    {
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        'base_simple/base_simple.gyp:*',
        'base_unittest/base_unittest.gyp:*',
      ],
    }, # target_name: All
  ], # conditions
}