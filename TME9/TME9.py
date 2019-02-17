import numpy as np
import matplotlib.pyplot as plt
import pickle as pkl


def tirage(m):
	return np.random.uniform(-m, m), np.random.uniform(-m, m)


def monteCarlo(n):
	coord = np.zeros((n, 2))
	for i in range(n):
		coord[i, :] = tirage(1.0)
	mc = (np.sum(np.multiply(coord, coord), axis=1) <= 1).astype(int)
	return (4.0 * np.sum(mc)) / n, coord[:, 0], coord[:, 1]


def swapF(tau):
	n = len(tau)
	c1, c2 = np.random.randint(n, size=2)
	while c1 == c2:
		c1, c2 = np.random.randint(n, size=2)
	keys = list(tau.keys())
	c1 = keys[c1]
	c2 = keys[c2]
	newtau = {}
	for k in keys:
		if k == c1:
			newtau[c1] = tau[c2]
		elif k == c2:
			newtau[c2] = tau[c1]
		else:
			newtau[k] = tau[k]
	return newtau


def decrypt(mess, tau):
	ds = ""
	for i in range(len(mess)):
		ds += tau[mess[i]]
	return ds


def logLikelihood(mess, mu, A, chars2index):
	last = chars2index[mess[0]]
	L = np.log(mu[last])
	for i in mess[1:]:
		L += np.log(A[last][chars2index[i]])
		last = chars2index[i]
	return L


def identityTau(count):
	tau = {}
	for k in list(count.keys()):
		tau[k] = k
	return tau


def MetropolisHastings(mess, mu, A, tau, N, chars2index):
	best = decrypt(mess,tau)
	bestL = logLikelihood(best, mu, A, chars2index)
	print(bestL, best)
	for i in range(N):
		newtau = swapF(tau)
		newmess = decrypt(mess, newtau)
		newL = logLikelihood(newmess, mu, A, chars2index)
		L = logLikelihood(decrypt(mess, tau), mu, A, chars2index)
		if newL >= L or (L - newL <= 100 and np.random.rand() <= 1/np.power(np.e, L-newL)):
			tau = newtau
			if newL > bestL:
				best = newmess
				bestL = newL
				print(bestL, best)
	return best


plt.figure()

# trace le carrÃ©
plt.plot([-1, -1, 1, 1], [-1, 1, 1, -1], '-')

# trace le cercle
x = np.linspace(-1, 1, 100)
y = np.sqrt(1 - x * x)
plt.plot(x, y, 'b')
plt.plot(x, -y, 'b')

# estimation par Monte Carlo
pi, x, y = monteCarlo(int(1e4))

# trace les points dans le cercle et hors du cercle
dist = x * x + y * y
plt.plot(x[dist <= 1], y[dist <= 1], "go")
plt.plot(x[dist > 1], y[dist > 1], "ro")
plt.show()

# si vos fichiers sont dans un repertoire "ressources"
with open("countWar.pkl", 'rb') as f:
	(count, mu, A) = pkl.load(f, encoding='latin1')

with open("secret.txt", 'r') as f:
	secret = f.read()[0:-1]  # -1 pour supprimer le saut de ligne

with open("secret2.txt", 'r') as f:
	secret2 = f.read()[0:-1]  # -1 pour supprimer le saut de ligne

chars2index = pkl.load(open("fichierHash.pkl", 'rb'))

# ATTENTION: mu = proba des caractere init, pas la proba stationnaire
# => trouver les caractÃ¨res frÃ©quents = sort (count) !!
# distribution stationnaire des caracteres
freqKeys = np.array(list(count.keys()))
freqVal  = np.array(list(count.values()))
# indice des caracteres: +freq => - freq dans la references
rankFreq = (-freqVal).argsort()

# analyse mess. secret: indice les + freq => - freq
cles = np.array(list(set(secret))) # tous les caracteres de secret2
rankSecret = np.argsort(-np.array([secret.count(c) for c in cles]))
# ATTENTION: 37 cles dans secret, 77 en gÃ©nÃ©ral... On ne code que les caractÃ¨res les plus frequents de mu, tant pis pour les autres
# alignement des + freq dans mu VS + freq dans secret
tau_init = dict([(cles[rankSecret[i]], freqKeys[rankFreq[i]]) for i in range(len(rankSecret))])

MetropolisHastings(secret, mu, A, tau_init, 50000, chars2index)
#MetropolisHastings( secret2, mu, A, identityTau (count), 10000, chars2index)
