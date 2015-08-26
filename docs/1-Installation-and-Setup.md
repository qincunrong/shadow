Shadow can either be run in a [traditional computing](#shadow-with-traditional-computing) environment, or in the [EC2 cloud](#shadow-with-cloud-computing) with a pre-configured machine instance.

# Shadow with Traditional Computing

## Installing Dependencies

**Required**:
  + gcc, gcc-c++
  + clang, llvm (version >= 3.2)
  + glib (version >= 2.32.0)
  + igraph (version >= 0.5.4)
  + cmake (version >= 2.8.8)
  + make
  + xz-utils

**Recommended Python Modules** (for helper/analysis scripts):
  + python, numpy, scipy, matplotlib, networkx, lxml

**Recommended System Tools**:
  + git, dstat, screen, htop

**YUM (Fedora):**

```bash
sudo yum install -y gcc gcc-c++ clang clang-devel llvm llvm-devel glib2 glib2-devel igraph igraph-devel cmake make xz
sudo yum install -y python numpy scipy python-matplotlib python-networkx python-lxml
sudo yum install -y git dstat screen htop
```

**APT (Ubuntu):**

```bash
sudo apt-get install -y gcc g++ clang llvm llvm-dev libglib2.0 libglib2.0-dev libigraph0 libigraph0-dev cmake make xz-utils
sudo apt-get install -y python python-matplotlib python-numpy python-scipy python-networkx python-lxml
sudo apt-get install -y git dstat screen htop
```

## Shadow Setup

```bash
git clone https://github.com/shadow/shadow.git -b release
cd shadow
./setup build
./setup install
```

You should add `/home/${USER}/.shadow/bin` to your shell setup for the PATH environment variable (e.g., in `~/.bashrc` or `~/.bash_profile`).

```bash
echo "export PATH=${PATH}:/home/${USER}/.shadow/bin" >> ~/.bashrc && bash
```

Check that Shadow is installed and runs:

```bash
shadow --version
shadow --help
```

## Setup Notes

+ All build output is generated out-of-source, by default to the `./build` directory.
+ Use `./setup build --help` to see all build options; the most useful build options are:  
 + `-g` or `--debug` to build Shadow with debugging symbols
 + `--include` and `--library` if you installed any dependencies in non-standard locations or somewhere other than `~/.shadow`. For example,  if you build llvm in `/opt/llvm`, then you need something like  
`./setup build --include=/opt/llvm-3.2/share/llvm/cmake --include=/opt/llvm-3.2/include`.
 + `--prefix` if you want to install Shadow somewhere besides `~/.shadow`
+ The `setup` script is a wrapper to `cmake` and `make`. Using `cmake` and `make` directly is also possible, but strongly discouraged.

## System Configs and Limits

Some Linux system configuration changes are needed to run large-scale Shadow simulations (more than about 1000 nodes).

### Number of Open Files

There is a default linux system limit on the number of open files. If each node 
in your Shadow plug-in opens many file or socket descriptors (if you have many nodes, this is very likely to happen), you'll likely want to increase the limit so you application doesn't start getting errors when calling `open()` or `socket()`.

#### System-wide Limits

Check the _system wide_ limits with:

```bash
sysctl fs.file-max
cat /proc/sys/fs/file-nr
```

The latter command shows you:
 1. the system-wide number of open file handles
 1. the system-wide number of free handles
 1. and the system-wide limit on the maximum number of open files for all processes

Change the limit, persistent across reboots, and apply now:

```bash
sysctl -w fs.file-max=5000000
echo "fs.file-max = 5000000" >> /etc/sysctl.conf
sysctl -p
```

#### User Limits

You will want to almost certainly want to raise the user file limit by modifying `/etc/security/limits.conf`. After modifying, you need to either log out and back in or reboot for the changes to take affect.

Check the maximum number of open file descriptors allowed in your _current session_:
```bash
ulimit -n
```

Update the user limit. For example, to handle all of our network configurations on EC2, I use:

```
* soft nofile 25000
* hard nofile 25000
```

You can watch `/proc/sys/fs/file-nr` and reduce the limit to something less than 25000 according to your usage, if you'd like.

For more information:

```bash
man proc
man ulimit -n
cat /proc/sys/fs/file-max
cat /proc/sys/fs/inode-max
```

### Number of Maps

There is a system limit on the number of `mmap()` mappings per process. Most users will not have to modify these settings. However, if an application running in Shadow makes extensive use of `mmap()`, you may need to increase the limit.

#### Process Limit

The process limit can be queried in these ways:

```bash
sysctl vm.max_map_count
cat /proc/sys/vm/max_map_count
```

You can check the number of maps currently used in a process with pid=PID like this:

```bash
cat /proc/PID/maps | wc -l
```

Set a new limit, make it persistent, apply it now:

```bash
sudo sysctl -w vm.max_map_count=262144
sudo echo "vm.max_map_count = 262144" >> /etc/sysctl.conf
sudo sysctl -p
```

For more information:
https://www.kernel.org/doc/Documentation/sysctl/vm.txt

# Shadow with Cloud Computing

Amazon’s [Elastic Compute Cloud (EC2)](http://aws.amazon.com/ec2/) infrastructure provides a simple and relatively [cost-efficient](http://aws.amazon.com/ec2/#pricing) way to run large-scale Shadow experiments without the need to buy expensive hardware or manage complex configurations. You can get started running Shadow experiments on EC2 in minutes using our pre-configured public EC2 AMI, which has already been set up to run Shadow.

1. Sign up for [Amazon EC2 access](https://aws-portal.amazon.com/gp/aws/developer/registration)
1. Launch an instance using our pre-installed and configured [Shadow-cloud AMI (ami-0f70c366)](https://console.aws.amazon.com/ec2/home?region=us-east-1#launchAmi=ami-0f70c366) based on Ubuntu LTS releases
1. Follow the New Instance Wizard
   + the **instance type** you’ll need depends on what size Shadow network you’ll want to simulate (see [the shadow-plugin-tor wiki](https://github.com/shadow/shadow-plugin-tor) for scalability estimates)
   + create and download a new **keypair** if you don’t already have one, since you’ll need it for SSH access
   + create a new **security group** for the Shadow-cloud server
   + configure the **firewall** to allow inbound SSH on 0.0.0.0/0
1. Once the instance is launched and ready, find the public DNS info and remotely log into the machine using the keypair you downloaded:
```bash
ssh -i your-key.pem ubuntu@your-public-dns.amazonaws.com
```
1. Once logged in, view `~/README` and `~/shadow-git-clone/README` for more information


# Shadow with Docker


## Pre-built images
A docker image that includes just Shadow (release v1.10.2) is available at [https://security.cs.georgetown.edu/shadow-docker-images/shadow-standalone.tar.gz](https://security.cs.georgetown.edu/shadow-docker-images/shadow-standalone.tar.gz).  

An image that includes Shadow along with the Tor plugin is available at [https://security.cs.georgetown.edu/shadow-docker-images/shadow-tor.tar.gz](https://security.cs.georgetown.edu/shadow-docker-images/shadow-tor.tar.gz).

To use these images:

1. Download the desired image (see above URLs).
2. Load it via: ```gunzip -c shadow-standalone.tar.gz | docker load```
3. Run it via: ```docker run -t -i -u shadow shadow-standalone /bin/bash```

(Replace *shadow-standalone* with *shadow-tor* if you are using the image with the Tor plugin.)

The **shadow** user has sudo access.  Its password is ***shadow***.

Remember to save your modifications!  See the fine [Docker documentation](https://docs.docker.com/articles/basics/) for more information about how to use containers and images.


## Building your own docker image

There are many ways to do this.  Below, we base our install off of Fedora 22.

1. Get the Fedora repository via ```docker pull fedora``` This may take a few minutes.
1. Start a container via ```docker run -i -t fedora:22 /bin/bash```
1. Create a non-root user: 

	```bash
	useradd -s /bin/bash -g wheel shadow
	dnf install -y passwd sudo
	passwd shadow			# set it to something sensible
	```
1. Install Shadow's dependencies (this may take a few minutes):

	```bash
	dnf install -y gcc gcc-c++ clang clang-devel llvm llvm-devel glib2 glib2-devel 	igraph igraph-devel cmake make xz
	dnf install -y python numpy scipy python-matplotlib python-networkx python-lxml
	dnf install -y git dstat screen htop
```
1. Become the shadow user: ```sudo -u shadow -s```
1. Change to the shadow user's home directory: ```cd```
1. Install Shadow:

	```bash
	git clone https://github.com/shadow/shadow.git -b release
	cd shadow
	./setup build
	./setup install
	```
1. You should add `/home/${USER}/.shadow/bin` to your shell setup for the PATH environment variable (e.g., in `~/.bashrc` or `~/.bash_profile`).

	```bash
	echo "export PATH=${PATH}:/home/${USER}/.shadow/bin" >> ~/.bashrc && bash
	```
1. (Optional) Install shadow-plugin-tor:
  + install the dependencies:
  
  	```bash
  	sudo dnf install -y gcc automake autoconf zlib zlib-devel gpg tar
 	```
 + compile and build shadow-plugin-tor:
 
 	```bash
 	git clone https://github.com/shadow/shadow-plugin-tor.git -b release
	cd shadow-plugin-tor
	./setup dependencies	# answer prompts
	./setup build	        # answer prompts
	./setup install
	```
1. Save the image:
  + First, note the name of your container.  You can find this via running ```docker ps``` **from another terminal** or, more simply, by looking at the prompt.  For example, if the prompt reads: ```[shadow@fffe28725f3f shadow]$```, then your container name is *fffe28725f3f*.
  + Exit both the shadow user's and root's shells via

	 ```bash
  	 exit
	 exit
	 # possibly run exit a third time if you ran bash during step 8 above
    ```
  + Commit the image via ```docker commit fffe28725f3f shadow-standalone```

You're done!  

If you want to create a new container using your new image, do:

```bash
docker run -t -i -u shadow shadow-standalone /bin/bash
```

If shadow isn't in your path, re-run the command from step 8.
