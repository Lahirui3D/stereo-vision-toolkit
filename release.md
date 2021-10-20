# Stereo Vision Toolkit v1.3.2

Changes:
- Remove computer restart from installer (still requires manual restart)

Known issues:
- Requires manual restart after installation.
- ROS perception YAML's data arrays must be on single line in YAML file.
- WLS filter causes speckle filter to sometimes be disabled.
- Stereo videos are saved uncompressed to avoid lossy compression which would cause issues when loading stereo videos. This will create large file sizes.