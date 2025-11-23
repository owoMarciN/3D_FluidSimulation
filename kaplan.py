import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.animation as animation

# Wał turbiny
class Base:
    #Konstruktor R - promień wału, H - wysokość wału
    def __init__(self,R,H):
        self.R = R
        self.H = H
    
    def plot(self):
        # Parametryzacja wału
        # t - kąt, h - wysokość z przedziału [0,H] 
        t = np.linspace(0,2 * np.pi, 100)
        h = np.linspace(0, self.H, 100)

        # Tworzenie siatki
        t,h = np.meshgrid(t,h)

        # Współrzędne x,y,z walca
        x_cylinder = self.R * np.cos(t)
        y_cylinder = self.R * np.sin(t)
        z_cylinder = h

        # Rysowanie wału
        ax.plot_surface(x_cylinder, y_cylinder, z_cylinder, zorder=1)

        
# Łopatka turbiny
class Turbine:
    #Konstruktor: t - kąt będzie należeć do przedziału [alpha,beta], d - szerokośc turbiny
    def __init__(self,alpha,beta,d,param=lambda t: t):
        self.alpha = alpha
        self.beta = beta
        self.d = d
        self.param = param

    # base - oznacza klasę Base (wał turbiny)
    def plot(self,base):
        # Parametryzacja łopatki turbiny
        # t - kąt z przedziału [alpha,beta], r - współrzędna radialna
        t = np.linspace(self.alpha, self.beta, 200)
        r = np.linspace(base.R, base.R + self.d, 200)

        # Tworzenie siatki
        t,r = np.meshgrid(t,r)

        condition =  4*(1/(self.alpha-self.beta))**2*(t-(self.alpha+self.beta)/2)**2 + (1/self.d)**2*(r-base.R)**2 < 1

        t = np.where(condition,t,np.nan)
        r = np.where(condition,r,np.nan)

        # Współrzędne x,y,z turbiny
        x_turbine = r*np.cos(t)
        y_turbine = r*np.sin(t)
        z_turbine = 0.2*base.H*(self.param(t-self.alpha) - self.param(0))/(self.param(self.beta-self.alpha) - self.param(0)) + 0.4*base.H

        ax.plot_surface(x_turbine, y_turbine, z_turbine, edgecolors = [1, 0, 0],zorder=2)


b = Base(R=3, H=3)

turbines = [Turbine(n*np.pi/4, 1+n*np.pi/4, d=3) for n in range(8)]


# Wykres 3D
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

b.plot()

for t in turbines:
    t.plot(b)

ax.set_axis_off()
ax.view_init(elev=10, azim=20)
#plt.savefig("turbina3d.png", dpi=1000)

ax.set_box_aspect([1, 1, 1]) 

plt.show()


