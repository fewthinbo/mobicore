#release
alias mobi_clean_rel "cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-release --target clean"
alias mobi_preset_rel "cd /usr/mobile/src/ && cmake --preset bsd-ninja-release"
alias mobi_install_rel "cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-release --target install"

#debug
alias mobi_clean_debug "cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-debug --target clean"
alias mobi_preset_debug "cd /usr/mobile/src/ && cmake --preset bsd-ninja-debug"
alias mobi_install_debug "cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-debug --target install"
