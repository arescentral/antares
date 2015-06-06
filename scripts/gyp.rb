# brew(1) formula for gyp.

require 'formula'

class Gyp < Formula
    homepage "https://code.google.com/p/gyp/"
    head "https://chromium.googlesource.com/external/gyp", :using => :git

    depends_on :python

    def install
        system 'python', 'setup.py', 'install'

        bin.install("gyp")
        bin.install("gyp_main.py")
    end
end
