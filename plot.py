import numpy as np
import matplotlib.pyplot as plt

def load_poses(filename):
    data = np.loadtxt(filename)
    # data shape: (N,7) [x,y,z,qx,qy,qz,qw]
    return data[:,0:3]

poses_before = load_poses("poses_before.txt")
poses_after = load_poses("poses_after.txt")

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# ax.plot(poses_before[:,0], poses_before[:,1], poses_before[:,2], label='Before Optimization', color='red')
ax.plot(poses_after[:,0], poses_after[:,1], poses_after[:,2], label='After Optimization', color='green')

ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.legend()
plt.show()