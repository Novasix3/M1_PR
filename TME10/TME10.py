import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

a = 6.
b = -1.
N = 100
sig = .4


def cov(X, Y):
	return np.mean(np.multiply(X, Y)) - np.mean(X) * np.mean(Y)


x = np.random.rand(N)
y = a * x + b + np.random.randn(N) * sig

plt.scatter(x, y)

covar = cov(x, y)
std = np.std(x)
a_proba = covar / (std * std)
b_proba = np.mean(y) - (covar * np.mean(x)) / (std * std)
predx = np.linspace(0, 1, N)
predy = a_proba * np.linspace(0, 1, N) + b_proba
plt.plot(predx, predy)
plt.show()

X = np.hstack((x.reshape(N, 1), np.ones((N, 1))))
A = np.dot(X.transpose(), X)
B = np.dot(X.transpose(), y.reshape(N, 1))
coeffs = np.linalg.solve(A, B)
plt.figure()
plt.scatter(x, y)
predy = coeffs[0] * np.linspace(0, 1, N) + coeffs[1]
plt.plot(predx, predy)
plt.show()

plt.figure()
wstar = np.linalg.solve(X.T.dot(X), X.T.dot(y))

eps = 5e-3
nIterations = 100
w = np.zeros(X.shape[1])  # init Ã  0
allw = [w]
for i in range(nIterations):
	w = w - eps * (np.dot(2 * X.transpose(), np.dot(X, w) - y))
	allw.append(w)
	print(w)

allw = np.array(allw)
# tracer de l'espace des couts
ngrid = 20
w1range = np.linspace(-0.5, 8, ngrid)
w2range = np.linspace(-1.5, 1.5, ngrid)
w1, w2 = np.meshgrid(w1range, w2range)

cost = np.array([[np.log(((X.dot(np.array([w1i, w2j])) - y) ** 2).sum()) for w1i in w1range] for w2j in w2range])

plt.figure()
plt.contour(w1, w2, cost)
plt.scatter(wstar[0], wstar[1], c='r')
plt.plot(allw[:, 0], allw[:, 1], 'b+-', lw=2)
plt.show()

costPath = np.array([np.log(((X.dot(wtmp) - y) ** 2).sum()) for wtmp in allw])
costOpt = np.log(((X.dot(wstar) - y) ** 2).sum())

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot_surface(w1, w2, cost, rstride=1, cstride=1)
ax.scatter(wstar[0], wstar[1], costOpt, c='r')
ax.plot(allw[:, 0], allw[:, 1], costPath, 'b+-', lw=2)
for spine in ax.spines.values():
	spine.set_visible(False)
plt.show()

# non linÃ©aire
c = 1
x = np.random.rand(N)
y = a * x * x + b * x + c + np.random.randn(N) * sig
plt.figure()
plt.scatter(x, y)
X = np.hstack(((x * x).reshape(N, 1), x.reshape(N, 1), np.ones((N, 1))))
wstar = np.linalg.solve(X.T.dot(X), X.T.dot(y))
xquad = np.linspace(0, 1, N)
yquad = wstar[0] * xquad * xquad + wstar[1] * xquad + wstar[2]
plt.plot(xquad, yquad)
plt.show()
ypred = wstar[0] * x * x + wstar[1] * x + wstar[2]
print(np.mean((ypred - y) * (ypred - y)))

data = np.loadtxt("winequality-red.csv", delimiter=";", skiprows=1)
N, d = data.shape  # extraction des dimensions
pcTrain = 0.7  # 70% des donnÃ©es en apprentissage
allindex = np.random.permutation(N)
indTrain = allindex[:int(pcTrain * N)]
indTest = allindex[int(pcTrain * N):]
X = data[indTrain, :-1]  # pas la derniÃ¨re colonne (= note Ã  prÃ©dire)
Y = data[indTrain, -1]  # derniÃ¨re colonne (= note Ã  prÃ©dire)
# Echantillon de test (pour la validation des rÃ©sultats)
XT = data[indTest, :-1]  # pas la derniÃ¨re colonne (= note Ã  prÃ©dire)
YT = data[indTest, -1]  # derniÃ¨re colonne (= note Ã  prÃ©dire)

w = np.linalg.solve(X.T.dot(X), X.T.dot(Y))
ypredtrain = X.dot(w)
ypredtest = XT.dot(w)
print("Lineaire")
print("Train : ", np.mean((ypredtrain - Y) * (ypredtrain - Y)))
print("Test : ", np.mean((ypredtest - YT) * (ypredtest - YT)))
print("Exact Train : ", np.mean(np.round(ypredtrain) == np.round(Y)))
print("Exact Test : ", np.mean(np.round(ypredtest) == np.round(YT)))

X = np.hstack((X * X, X))
XT = np.hstack((XT * XT, XT))

w = np.linalg.solve(X.T.dot(X), X.T.dot(Y))
ypredtrain = X.dot(w)
ypredtest = XT.dot(w)
print("Quadratique")
print("Train : ", np.mean((ypredtrain - Y) * (ypredtrain - Y)))
print("Test : ", np.mean((ypredtest - YT) * (ypredtest - YT)))
print("Exact Train : ", np.mean(np.round(ypredtrain) == np.round(Y)))
print("Exact Test : ", np.mean(np.round(ypredtest) == np.round(YT)))

a = 6.
b = -1.
N = 100
sig = .4
x = np.random.rand(N)
y = a * x + b + np.random.randn(N) * sig
X = np.hstack((x.reshape(N, 1), np.ones((N, 1))))
eps = 2e-2
nIterations = 1000
w = np.zeros(X.shape[1])  # init Ã  0
allw = [w]
for i in range(nIterations):
	index = np.random.randint(0, N)
	xi = X[index]
	yi = y[index]
	w = w - eps * np.dot((2 * xi.T), (np.dot(xi, w) - yi))
	allw.append(w)
	print(w)

allw = np.array(allw)
# tracer de l'espace des couts
ngrid = 20
w1range = np.linspace(-0.5, 8, ngrid)
w2range = np.linspace(-1.5, 1.5, ngrid)
w1, w2 = np.meshgrid(w1range, w2range)

cost = np.array([[np.log(((X.dot(np.array([w1i, w2j])) - y) ** 2).sum()) for w1i in w1range] for w2j in w2range])
wstar = np.linalg.solve(X.T.dot(X), X.T.dot(y))
plt.figure()
plt.contour(w1, w2, cost)
plt.scatter(wstar[0], wstar[1], c='r')
plt.plot(allw[:, 0], allw[:, 1], 'b+-', lw=2)
plt.show()
