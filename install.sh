#!/bin/bash

SRC=~/Shadonix/src
cd $SRC && cd libshado/ && make clean && make && make install &&
cd $SRC && cd libfb/ && make clean && make && make install &&
cd $SRC && cd init/ && make clean && make && make install &&
cd $SRC && cd lash/ && make clean && make && make install &&
cd $SRC && cd print_args/ && make clean && make && make install &&
cd $SRC && cd fbset/ && make clean && make && make install &&


sudo mount ~/Shadonix/shadonix_rootfs /mnt/Shadonix -t ext4 -o loop &&
sudo rsync -vr --progress ~/Shadonix/* /mnt/Shadonix --exclude shadonix_rootfs &&
sudo rm /mnt/Shadonix/LICENSE /mnt/Shadonix/README.md /mnt/Shadonix/install.sh &&
sudo umount /mnt/Shadonix &&
qemu-system-x86_64 -m 1024 -boot d -enable-kvm -smp 3 -hda ~/Shadonix/shadonix_rootfs -kernel ~/Shadonix/boot/vmlinuz-linux -initrd ~/Shadonix/boot/initramfs-linux.img -serial stdio -append "root=/dev/sda console=ttyS0 panic=0"
