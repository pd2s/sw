# sw_swaybar

sw_swaybar is almost feature-compatible with original swaybar.

# Key differences between sw_swaybar and swaybar
* sw_swaybar supports tray dbusmenu, swaybar doesnt.
* For sw_swaybar, fonts must be specified in fontconfig pattern instead of pango font description as in original swaybar.

# Example sway config file
```
bar {
    swaybar_command sw_swaybar
    font monospace:size=16
    ...
}
```
