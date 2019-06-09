#!/bin/bash
# Huaicheng Li <huaicheng@cs.uchicago.edu>
# Run VM with lightnvm support

IMGDIR=$HOME/images

is_mounted=$(mount | grep "/mnt/tmpfs")

if [[ $is_mounted == "" ]]; then
    sudo mkdir -p /mnt/tmpfs
    # huge=always
    sudo mount -t tmpfs -o size=128G,huge=always tmpfs /mnt/tmpfs
fi


# every time we run a new SSD
sudo rm -rf /mnt/tmpfs/test1.raw

[[ ! -e /mnt/tmpfs/test1.raw ]] && ./qemu-img create -f raw /mnt/tmpfs/test1.raw 128G

# huge page related settings
#echo 25000 | sudo tee /proc/sys/vm/nr_hugepages

[[ ! -d /dev/hugepages2M ]] && sudo mkdir /dev/hugepages2M && sudo mount -t hugetlbfs none /dev/hugepages2M -o pagesize=2M


#-object iothread,id=iothread0 \
#-device virtio-blk-pci,iothread=iothread0,drive=id0 \
    #-nographic \
    #-device nvme,drive=id0,serial=serial0,id=nvme0 \
    #-kernel /home/huaicheng/git/linux/arch/x86_64/boot/bzImage \
    #-append "root=/dev/vda1 console=ttyS0,115200n8 console=tty0" \
    #-virtfs local,path=/home/huaicheng/share/,security_model=passthrough,mount_tag=host_share \

    #must come before all other qemu options!!!!!!
    #-trace events=/tmp/events \
    #-object memory-backend-file,id=mem1,size=8G,mem-path=/dev/hugepages2M \
    #-device pc-dimm,id=dimm1,memdev=mem1 \

    # VOC related options
    #lbbtable=/media/bbtable.qemu,


sudo valgrind x86_64-softmmu/qemu-system-x86_64 \
    -name "nvme-FEMU-test" \
    -enable-kvm \
    -cpu host \
    -smp 16 \
    -m 8G,slots=2,maxmem=32G \
    -object memory-backend-file,id=mem1,size=8G,mem-path=/dev/hugepages2M \
    -device pc-dimm,id=dimm1,memdev=mem1 \
    -device virtio-scsi-pci,id=scsi0 \
    -device scsi-hd,drive=hd0 \
    -drive file=$IMGDIR/u14s.qcow2,if=none,aio=native,cache=none,format=qcow2,id=hd0 \
    -device virtio-scsi-pci,id=scsi1 \
    -device scsi-hd,drive=hd1 \
    -drive file=$IMGDIR/vmdata.qcow2,if=none,aio=native,cache=none,format=qcow2,id=hd1 \
    -drive file=/mnt/tmpfs/test1.raw,if=none,aio=threads,format=raw,id=id0 \
    -device nvme,drive=id0,serial=serial0,id=nvme0,namespaces=1,lver=1,lmetasize=16,ll2pmode=0,nlbaf=5,lba_index=3,mdts=10,lnum_ch=16,lnum_lun=8,lnum_pln=2,lsec_size=4096,lsecs_per_pg=4,lpgs_per_blk=512,lbbtable=/media/bbtable.qemu,lmetadata=/media/meta.qemu,ldebug=0 \
        -net user,hostfwd=tcp::8080-:22 \
    -net nic,model=virtio \
    -nographic \
    -qmp unix:./qmp-sock,server,nowait | tee /media/log
    #-object iothread,id=iothread0 \
    #-display none \
    #-nographic \
    #-monitor stdio \
    #-s -S \
    #

#sleep 10

#./pin.sh
#sshsim "~/tsc.sh"
#sshsim "echo 0 | sudo tee /proc/sys/kernel/timer_migration"
#sshsim "echo 0 | sudo tee /sys/kernel/debug/tracing/tracing_on"

echo "VM is up, enjoy it :)"

wait
