import argparse
import os
import subprocess
import time
import signal


def bench_run():

    print("running bench mark")
    drop_db = "drop database testdb;"
    create_db = "create database testdb;"
    mysql_str = "mysql -u root -p1234"
    p1 = subprocess.Popen(mysql_str.split(),stdin=subprocess.PIPE)
    p1.communicate(drop_db.encode())
    p1.wait()
    p2 = subprocess.Popen(mysql_str.split(),stdin=subprocess.PIPE)
    p2.communicate(create_db.encode())
    p2.wait()
    
    file = open("rmdb_client/build/a.sql",'r')
    
    p3= subprocess.Popen(mysql_str.split(),stdin=file,stdout=subprocess.PIPE)
    output,errors = p3.communicate()
    p3.wait()
    result = output.decode().splitlines()
    res_file  = open("rmdb_client/build/a.txt","w")
    for line in result:
        res_file.write('|')
        for word in line.split():
            res_file.write(' '+word+' |')
        res_file.write('\n')
    res_file.close()
            

def my_run():

    print("running rmdb")
    os.chdir('/home/one/db2023/build')
    if os.path.exists('testdb'):
        import shutil
        shutil.rmtree('testdb', ignore_errors=True)
    svr = "./bin/rmdb testdb"
    p1 = subprocess.Popen(svr.split(),preexec_fn=os.setsid)
    time.sleep(10)
    os.chdir("/home/one/db2023")
    client = "./rmdb_client/build/rmdb_client"
    file = open("./rmdb_client/build/a.sql",'r')
    
    p2= subprocess.Popen(client.split(),stdin=file,stdout=subprocess.PIPE)
    output,errors = p2.communicate()
    p2.wait()
    p1.send_signal(signal.SIGINT)
    

def build():
    # build svr

    print("building rmdb...")
    if not os.path.exists("./build") :
        os.makedirs("./build")

    if not os.path.exists("./build/Makefile"):
        os.chdir("./build")
        subprocess.run(['cmake','..','-DCMAKE_EXPORT_COMPILE_COMMANDS=1'])
        os.chdir("..")


    os.chdir("./build")
    subprocess.run(['make','-j20'])
    os.chdir("..")
    

    print("building rmdb client")
    if not os.path.exists("./rmdb_client/build"):
        os.makedirs("./rmdb_client/build")
    if not os.path.exists("./rmdb_client/build/Makefile"):
        os.chdir("./rmdb_client/build")
        subprocess.run(['cmake','..','-DCMAKE_EXPORT_COMPILE_COMMANDS=1'])
        os.chdir("../..")
    os.chdir("./rmdb_client/build")
    subprocess.run(['make','-j20'])
    
    os.chdir("../..")






def main():

    
    parser = argparse.ArgumentParser()
    parser.add_argument("-b","--build",help = '编译项目',action='store_true')
    parser.add_argument("-r","--run",help = '运行测试',action='store_true')

    args = parser.parse_args()
    if args.build:
        build()
    if args.run:
        #bench_run()
        my_run()
    return 0


if __name__ == '__main__':
    main()
    