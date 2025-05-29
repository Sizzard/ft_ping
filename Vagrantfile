Vagrant.configure("2") do |config|
  config.vm.box = "debian/bookworm64"
  config.vm.provider "virtualbox" do |vb|
    # Display the VirtualBox GUI when booting the machine
    vb.gui = false
  end

  config.vm.define "ft-ping-42" do |control|
    control.vm.hostname = "ft-ping-42"
    control.vm.network "private_network", ip: "192.168.56.110"
    control.vm.provider "virtualbox" do |vb|
      vb.customize ["modifyvm", :id, "--name", "ft-ping-42"]
      vb.memory = "4096"
      vb.cpus = 4
    end
    control.vm.provision "shell", path: "./setup.sh"
  end
end