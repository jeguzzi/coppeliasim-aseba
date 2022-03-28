# coppeliasim-aseba
Simulate specific robots (Thymio) and generic Aseba nodes in CoppeliaSim


### IR sensors

#### Response


Reflectivity $r$ as a function of material color
double toGray() const { return (components[0] + components[1] + components[2]) / 3; }

```math
v_{\textrm{gray}}(r, g, b) = \frac{r + g + b}{3}\\
v_{\textrm{red}}(r, g, b) = 0.8 r + 0.2
```
```math

r(v) =  \frac{1}{1 + e^{-\frac{v - v_0}{\beta} }} \\
v_0 =  0.44\\
\beta = 0.11
```



Reflected light intensity $I$ as a function of distance $d$, reflectivity $r$ and incidence angle $\alpha$:
```math
I = r \cos(\alpha) \left(\frac{\lambda - \delta}{d - \delta}\right)^2 \\
\delta_{\textrm{h}} = 0.3 mm\\
\lambda_{\textrm{h}} = 85.7 mm\\
\delta_{\textrm{h}} = \\
\lambda_{\textrm{h}} = \\
```

Response vs intensity:
```math
y = y_{\max} \left(1 + \frac{1}{I}\right)^{-1}
\y_{\max}{\textrm{h}} = 4505\\
\y_{\min}{\textrm{h}} = 1000\\
\y_{\max}{\textrm{v}} = 1023\\
\y_{\min}{\textrm{v}} = 0\\
```
(if y < y_{\min}) y = 0

<!-- ![equation](http://www.sciweavers.org/tex2img.php?eq=I%20%3D%20%5Cleft%28%5Cfrac%7B%5Clambda%20-%20%5Cdelta%7D%7Bd%20-%20%5Cdelta%7D%5Cright%29%5E2&bc=White&fc=Black&im=jpg&fs=12&ff=arev&edit=0) -->
