$mountpoint = "/media/pi".freeze

dirs = Dir.glob("#{$mountpoint}/*")

dirs.delete_if {|d|
  s = File.stat(d)
  s.file?
}
  

if( dirs.empty? )
  puts "no usb disk";
end


dir = dirs[0];
puts("usb disk select #{dir}");

log =  Time.now.strftime("%Y%m%dT%H%M%S")


abspath = dir + "/" + log + ".log"
puts("file is #{abspath}");

system( "./switchlog #{abspath}");


