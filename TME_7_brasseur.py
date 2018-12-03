import numpy as np
import matplotlib.pyplot as plt
import pickle as pkl

# truc pour un affichage plus convivial des matrices numpy
np.set_printoptions(precision=2, linewidth=320)
plt.close('all')


# affichage d'une lettre
def tracerLettre(let):
	a = -let * np.pi / 180;  # conversion en rad
	coord = np.array([[0, 0]]);  # point initial
	for i in range(len(a)):
		x = np.array([[1, 0]]);
		rot = np.array([[np.cos(a[i]), -np.sin(a[i])], [np.sin(a[i]), np.cos(a[i])]])
		xr = x.dot(rot)  # application de la rotation
		coord = np.vstack((coord, xr + coord[-1, :]))
	plt.figure()
	plt.plot(coord[:, 0], coord[:, 1])
	plt.savefig("exlettre.png")
	plt.show()
	return


def discretise(X, d):
	return np.floor(np.array(X) / (360. / d)).astype(int)


def initGD(X, N):
	GD = []
	for x in X:
		GD.append(np.floor(np.linspace(0, N - .00000001, len(x))).astype(int))
	GD = np.array(GD)
	return GD


def learnHMM(allx, alls, N, K, initTo0=False):
	if initTo0:
		A = np.zeros((N, N))
		B = np.zeros((N, K))
		Pi = np.zeros(N)
	else:
		eps = 1e-8
		A = np.ones((N, N)) * eps
		B = np.ones((N, K)) * eps
		Pi = np.ones(N) * eps

	for x in alls:
		Pi[x[0]] += 1
		last = x[0]
		for y in x[1:]:
			A[last][y] += 1
			last = y
	A = A / np.maximum(A.sum(1).reshape(N, 1), 1)  # normalisation
	Pi = Pi / Pi.sum()

	for i in range(len(allx)):
		for j in range(len(allx[i])):
			indexs = alls[i][j]
			indexx = allx[i][j]
			B[indexs][indexx] += 1
	B = B / np.maximum(B.sum(1).reshape(N, 1), 1)  # normalisation
	return Pi, A, B


def viterbi(x, Pi, A, B):
	d = np.zeros((len(x), len(Pi)))
	p = np.zeros((len(x), len(Pi)))
	for i in range(len(Pi)):
		d[0][i] = np.log(Pi[i]) + np.log(B[i][x[0]])
		p[0][i] = -1
	for t in range(1, len(x)):
		for j in range(len(Pi)):
			imax = 0
			for i in range(1, len(Pi)):
				if d[t - 1][i] + np.log(A[i][j]) > d[t - 1][imax] + np.log(A[imax][j]):
					imax = i
			d[t][j] = d[t - 1][imax] + np.log(A[imax][j]) + np.log(B[j][x[t]])
			p[t][j] = imax
	s = np.zeros(len(x)).astype(int)
	s[len(x) - 1] = np.argmax(d[len(x) - 1, :])
	T = len(x)
	for t in range(2, T):
		s[T - t] = p[T - t + 1][s[T - t + 1]]
	return s, np.max(d[len(x) - 1, :])


def calc_log_pobs_v2(x, Pi, A, B):
	T = len(x)
	J = len(Pi)
	a = np.zeros((T, J))
	for i in range(J):
		a[0][i] = Pi[i] * B[i][x[0]]
	for t in range(1, T):
		for j in range(J):
			for i in range(J):
				a[t][j] += a[t - 1][i] * A[i][j]
			a[t][j] *= B[j][x[t]]
	p = 0
	for i in range(J):
		p += a[T - 1][i]
	return np.log(p)


def baumWelsh(Xd, Y, N, K, initTo0=False):
	print("Y = ",Y)
	L = len(Y)
	A = np.zeros((L, N, N))
	B = np.zeros((L, N, K))
	Pi = np.zeros((L, N))
	HS = []

	for l in range(L):
		HS.append(initGD(Xd, N)[Y[l]])

	V = [-np.inf]
	while True:
		V.append(0)
		for l in range(L):
			Pi[l], A[l], B[l] = learnHMM(Xd[Y[l]], HS[l], N, K, initTo0)
			numX = len(Xd[Y[l]])
			for x in range(numX):
				HS[l][x], p = viterbi((Xd[Y[l]])[x], Pi[l], A[l], B[l])
				V[len(V) - 1] += p
				#V.append(p)
				#print("p = ",p)
		if (V[len(V) - 2] - V[len(V) - 1]) / V[len(V) - 2] < 0.0001:
			break
		plt.plot(range(0, len(V)), V)
		plt.show()
	return Pi, A, B


# separation app/test, pc=ratio de points en apprentissage
def separeTrainTest(y, pc):
	indTrain = []
	indTest = []
	for i in np.unique(y):  # pour toutes les classes
		ind, = np.where(y == i)
		n = len(ind)
		indTrain.append(ind[np.random.permutation(n)][:int(np.floor(pc * n))])
		indTest.append(np.setdiff1d(ind, indTrain[-1]))
	return indTrain, indTest


def probaSequence(s, Pi, A):
	p = Pi[s[0]]
	last = s[0]
	for step in s[1:]:
		p *= A[last][step]
		last = step
	if p > 0:
		return np.log(p)
	return -np.inf


def randWithProba(law):
	t = np.random.random_sample()
	for i in range(len(law)):
		if law[i] > t:
			return i
	return len(law)-1


def generateHMM(Pic, Ac, Bc, long):
	s = []
	x = []
	last = randWithProba(Pic)
	s.append(last)
	x.append(randWithProba(Bc[last]))
	for i in range(long - 1):
		last = randWithProba(Ac[last])
		s.append(last)
		x.append(randWithProba(Bc[last]))
	return s, x


with open('TME6_lettres.pkl', 'rb') as f:
	data = pkl.load(f, encoding='latin1')
X = np.array(data.get('letters'))  # récupération des données sur les lettres
Y = np.array(data.get('labels'))  # récupération des étiquettes associées

nCl = 5
K = 10
Xd = []
for x in X:
	Xd.append(discretise(x, K))  # application de la discrétisation
Xd = np.array(Xd)
Pi, A, B = learnHMM(Xd[Y == 'a'], initGD(Xd, nCl)[Y == 'a'], nCl, K, True)
s_est, p_est = viterbi(Xd[0], Pi, A, B)
print(s_est)
print(p_est)
print(calc_log_pobs_v2(Xd[0], Pi, A, B))

itrain, itest = separeTrainTest(Y, 0.8)

print("itrain = ",itrain)
Pi, A, B = baumWelsh(Xd, itrain, nCl, K, True)
models = []
for i in range(len(Pi)):
	models.append((Pi[i], A[i], B[i]))

ia = []
for i in itrain:
	ia += i.tolist()
it = []
for i in itest:
	it += i.tolist()

proba = np.array(
	[[calc_log_pobs_v2(Xd[i], models[cl][0], models[cl][1], models[cl][2]) for i in it] for cl in range(len(np.unique(Y)))])

Ynum = np.zeros(Y.shape)
for num, char in enumerate(np.unique(Y)):
	Ynum[Y == char] = num
pred = proba.argmax(0)  # max colonne par colonne
Ynum = Ynum[it].astype(int)
print(np.where(pred != Ynum, 0., 1.).mean())
conf = np.zeros((26, 26))
for i in range(len(pred)):
	conf[pred[i]][Ynum[i]] += 1

plt.figure()
plt.imshow(conf, interpolation='nearest')
plt.colorbar()
plt.xticks(np.arange(26), np.unique(Y))
plt.yticks(np.arange(26), np.unique(Y))
plt.xlabel(u'Vérité terrain')
plt.ylabel(u'Prédiction')
plt.savefig("mat_conf_lettres.png")
plt.show()


# Trois lettres générées pour 5 classes (A -> E)
n = 3  # nb d'échantillon par classe
nClred = 5  # nb de classes à considérer
d = K
fig = plt.figure()
for cl in range(nClred):
	Pic = models[cl][0].cumsum()  # calcul des sommes cumulées pour gagner du temps
	Ac = models[cl][1].cumsum(1)
	Bc = models[cl][2].cumsum(1)
	long = np.floor(
		np.array([len(x) for x in Xd[itrain[cl]]]).mean())  # longueur de seq. à générer = moyenne des observations
	print(long)
	for im in range(n):
		s, x = generateHMM(Pic, Ac, Bc, int(long))
		intervalle = 360. / d  # pour passer des états => angles
		newa_continu = np.array([i * intervalle for i in x])  # conv int => double
		sfig = plt.subplot(nClred, n, im + n * cl + 1)
		sfig.axes.get_xaxis().set_visible(False)
		sfig.axes.get_yaxis().set_visible(False)
		tracerLettre(newa_continu)
plt.savefig("lettres_hmm.png")
