# 服务脚本和启动命令都放在`/opt/data/pic_common 下`
# 安装pip2的方式
```
wget --no-check-certificate https://bootstrap.pypa.io/ez_setup.py
sudo python ez_setup.py --insecure

wget https://pypi.python.org/packages/11/b6/abcb525026a4be042b486df43905d6893fb04f05aac21c32c638e939e447/pip-9.0.1.tar.gz#md5=35f01da33009719497f01a4ba69d63c9
tar -xf pip-9.0.1.tar.gz
cd pip-8.0.0
sudo python setup.py install


//ln -s /usr/local/python27/bin/pip /usr/bin/pip
```

# 添加python所需要的依赖包
```
sudo pip2 install redis

///yum install python-setuptools
sudo easy_install supervisor
```

# lighttpd启动报错`/opt/data/pic_common/lib/modules/mod_compress.so libbz2.so.1.0: cannot open shared object file: No such file`
```
sudo ln -sf /lib64/libbz2.so.1.0.6 /lib64/libbz2.so.1.0
```

# supervisord log 不可写问题
```
sudo chown -hR svc:svc error.log
sudo chown -hR svc:svc debug.log
sudo chmod 777 debug.log
sudo chmod 777 error.log
```