{
  'targets': [
    {
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:*',
        '../sample/base.gyp:*',
        '../base_ex/base_ex.gyp:*',
      ],
    }, # target_name: All
  ], # conditions
}