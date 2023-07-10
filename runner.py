import argparse
import os
import subprocess


def remote_run():
    ssh_str = "ssh nao"
    mysql_str = "mysql -u root -p1234 < drop.sql"

    subprocess.run([ssh_str,mysql_str])
    
    

def build():
    # build svr

    print("building rmdb...")
    os.chdir("./build")
    svr_build_result =  subprocess.run(['make','-j20'],capture_output=True,text=True)
    print(svr_build_result.stderr)
    print(svr_build_result.stdout)
    os.chdir("..")

    print("building rmdb client")
    os.chdir("./rmdb_client/build")
    svr_build_result =  subprocess.run(['make','-j20'],capture_output=True,text=True)
    print(svr_build_result.stderr)
    print(svr_build_result.stdout)
    os.chdir("../..")






def main():

    
    parser = argparse.ArgumentParser()
    parser.add_argument("-b","--build",help = '编译项目',action='store_true')
    parser.add_argument("-r","--run",help = '运行测试',action='store_true')

    args = parser.parse_args()
    if args.build:
        build()
    if args.run:
        remote_run()
    return 0


if __name__ == '__main__':
    main()
    