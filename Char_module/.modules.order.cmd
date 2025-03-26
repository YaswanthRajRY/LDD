cmd_/home/ubuntu/LDD/Char_module/modules.order := {   echo /home/ubuntu/LDD/Char_module/char_module.ko; :; } | awk '!x[$$0]++' - > /home/ubuntu/LDD/Char_module/modules.order
