{
  'targets': [
    {
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        '../base/base.gyp:*',
        '../base_ex/base_ex.gyp:*',
        '../sample/base.gyp:*',
        '../sample/message_loop_mfc/message_loop_mfc.gyp:*',
      ],
    }, # target_name: All
  ], # conditions
}