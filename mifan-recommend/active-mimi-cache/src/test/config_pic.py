# config_pic.py

configs = {
    'cached': {
        'host': '127.0.0.1',
        'port': 6698,
        'password': 'ta0mee@123',
        'check_queue': 'image:check:queue',
        'retry_queue': 'image:retry:queue',
        'timeout': 5
    },
    'server': {
        'url': 'http://10.30.1.213:5000/yidun/image/check',
        'code': {
            'succ': 0,
            'retry': 530
        },
        'result': {
            'pass': 0,
            'not_sure': 2,
            'illegal': 1
        }
    },
    'cdn': {
        'get': 'http://img.mifan.61.com/',
    	'refresh': 'http://10.30.170.3:8000/cdn/?url=',
        'callback': {
            'root':'http://uploadimg.mifan.61.com:8081/callback.fcgi?',
            'path':'image_name=',
            'op':'&manual_review_result='
        }
    },
    'picture': {
    	'root': '/opt/data/www/',
    	'fail': '/opt/data/error_img/',
    	'square': '_128x128'
    },
    'params': {
        'id': '201811290689',
        'key': '77f7d5b13aec17b7b7770e1a92a2a0f5',
        'game_id': 689
    }
}
