#!/bin/bash

# filename   : emdwl-start.sh
# created at : 2023-01-14 20:02:17
# author     : xiliuya <xiliuya@163.com>

emacs --daemon=emdwl -q -l pos-tip.el -l emdwl-tool.el -f emdwl-tool-start
