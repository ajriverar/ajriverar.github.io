library(elasticnet) 	# lasso
library(gbm)   			# gbm (Stochastic Gradient Boosting)
library(kernlab)  		# svmPoly
library(randomForest) 	# rf, qrf
library(party) 			# cforest
library(caret)			# Main library to train and obtain relative importances
library(xgboost) 		# xgbLinear
library(bartMachine) 	# bartMachine
library(forecast)		# Series manipulation
library(monomvn) 		# bridge, blasso
library(monmlp) 		# monmlp
library(scmamp) 		# statistical tests


#################################
# ######## Exploratory analysis
##################################
# data: dataframe with the problem data. Each column is feature or predictor variable. The depedent variable is allocated in the output column 

#Basic exploratory analysis of the problem data (Min., Max., Mean, Median, ...) 
summary(data$output) 

# Correlation analysis between arrivals variable versus days of the week
boxplot(Arrivals ~ Weekday, xaxt="n",data = data, xlab="Weekday", ylab = "Arrivals in ED")
axis(1, at = 1:7, labels = c("Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"))

# Arrival series definition
arrivalSeries = ts(data$Arrivals)  

# Arrival series decomposition
decomp = decompose(arrivalSeries) 
plot(decomp)

# Dickey-fuller test to study stationarity
adfTest = adf.test(arrivalSeries)  

# Autocorretation function of Arrival series
p = forecast::Acf(arrivalSeries, lag.max = 21)


#################################
# ######## Ensemble computation of relative importance 
##################################
# dataTrain: rows of data (dataframe) corresponding to train data
# dataTest: rows of data (dataframe) corresponding to test data

MakeDataset<-function(data, parameters)
{
  # data: dataframe with the problem data. Each column is feature or predictor variable. The depedent variable is allocated in the output column 
  # parameters: parameters file with three rows
  # 	1 row: columns involved in the prediction, the last one the variable to predict and the previous ones the inputs taken with the delays in row 2. 
  # 	predict and the previous ones the inputs to be taken with the delays of row 2.
  # 	2nd row: lags to be considered, the last one is the horizon. E.g.: -5,-3,-1,0,1
  # 	3er row: columns of the data set for which the value of the corresponding delay is to be entered.
  # 	plus the horizon. Here it is not possible to enter the variable to be predicted, in our case 7. E.g.: 5,6 
  #
  # Returns a dataset of instances to train methods from the dataframe (data) with the problem data

  if (dim(parameters)[1] == 3) {varHorBool = TRUE
  } else {varHorBool = FALSE}
  nAttrib = length(which(!is.na(parameters[1,])))
  nData = dim(data)[1]
  nLags = length(which(!is.na(parameters[2,])))
  if (varHorBool) {nAttribHor = length(which(!is.na(parameters[3,])))
  } else {nAttribHor = 0}
  attribs = as.integer(parameters[1,1:nAttrib])
  lags = as.integer(parameters[2,1:nLags])
  if (varHorBool) {attribsHor = as.integer(parameters[3,1:nAttribHor])
  } else {attribsHor = c()}
  hLag = -min(lags)
  hor = lags[length(lags)]
  nITrain = nData-(hor+hLag);
  dataTrain = data.frame() 
  firstInstance = hLag
  for (j in 1:nITrain){   
    auxS = c()  
    for (h in 1:nAttrib){
      for (k in 1:(nLags-1)){  
        auxS = cbind(auxS, data[lags[k] + firstInstance + j, attribs[h]])
      }
    }  
    
    if (varHorBool){
        for (m in 1:nAttribHor){ 
          auxS = cbind(auxS, data[lags[nLags] + firstInstance + j, attribsHor[m]])
        }
      }
    
    auxS = cbind(auxS, data[lags[k+1] + firstInstance + j, attribs[h]])
    dataTrain=rbind(dataTrain,auxS)
  }
   
  headD = c() 
  for (h in attribs){
    for (k in lags[-length(lags)]){   
      headD = cbind(headD, paste(names(data)[h], k))
    }
  }
  
  if (varHorBool){
    for (m in attribsHor){
      headD = cbind(headD, paste(paste(names(data)[m], "Hor"), 0))
    }
  }
  
  headD = cbind(headD, "output") 
  names(dataTrain) = headD
  return(dataTrain)
}  

# Building the training and test datasets of instances
dataInstancesTrain = MakeDataset(dataTrain, parameters)
dataInstancesTest = MakeDataset(dataTest, parameters)

# Set the regression methods to use in the ensemble
regressionMethods <- c('bartMachine','lasso', 'svmPoly', 'monmlp','bridge','rf','blassoAveraged','qrf','xgbLinear','cforest')

#Training models
cvfolds = createFolds(dataInstancesTrain$output, k = 5, list = TRUE, returnTrain = TRUE) # con returnTrain a true, se consigue que luego con el modelo$pred$resample están las predicciones de test
trainParameter =  trainControl(method = "cv", index = cvfolds, savePredictions = 'final')
modelList = list()
for (r in regressionMethods){ 
  modelObtained <- caret::train(output~ ., data = dataInstancesTrain, method = r, trControl = trainParameter)
  modelList[[r]] <- modelObtained 
}

# Obtaining errors of methods in test datasets
maeList = list()
rmseList = list()
mapeList = list()
statisticList = list()
for (r in regressionMethods){ 

  valPred = caret::predict.train(modelList[[r]], newdata = dataInstancesTest)

  maeTest = mean(abs(valPred-dataInstancesTest$output))
  maeList[[r]] = maeTest
  
  rmse = sqrt(mean((valPred-dataInstancesTest$output)^2))
  rmseList[[r]] = rmse
  
  mape = mean(abs((valPred-dataInstancesTest$output)/dataInstancesTest$output))*100
  mapeList[[r]] = mape
}


# Obtaining the variable importance for each method
importanceAux = varImp(modelList[[1]], scale = FALSE)
importanceAux = data.frame(names = gsub("`",'',rownames(importanceAux$importance)), overall = importanceAux$importance$Overall)importanceAux = importanceAux[order(importanceAux$overall, decreasing = T),] # se ordenan descendentemente por valor de importance en las variables
importances = importanceAux

for (r in regressionMethods[-1]) {
  importanceAux = varImp(modelList[[r]], scale = FALSE)
  importanceAux = data.frame(names = gsub("`",'',rownames(importanceAux$importance)), overall = importanceAux$importance$Overall)
  importanceAux = importanceAux[order(importanceAux$overall, decreasing = T),] 
  importances = cbind(importances, importanceAux)
}
write.table(importances, file = 'importanciabyMethod.csv',  sep = ";", dec = ".", row.names = FALSE)

# Ensemble computation of relative importance
nModelos = length(regressionMethods)
positions = data.frame()
for (i in 1:nrow(importance)) {
  variableName = as.character(importance[i,1]) 
  pos = which(importance == variableName, arr.ind = TRUE) 
  mediaRanking = sum(pos[,1]) # 
  mediaRanking = mediaRanking/nModelos          
  SDRanking = sd(pos[,1]) 
  CVRanking = SDRanking / mediaRanking
  ICRanking = SDRanking / (dim(importance)[1]/2)
  positions = rbind(positions, data.frame(variableName, mediaRanking, SDRanking, CVRanking, ICRanking)) 
}
orderedPositions = positions[order(positions$mediaRanking),] # XAIRE Ranking of relative importance
write.table(orderedPositions, file = 'relativeImportance.csv',  sep = ";", dec = ".", row.names = FALSE) 

#################################
# ######## Computation of significant differences
##################################

# Load data
importances <- read.table("importanceByMethod.csv", sep = ";", as.is = TRUE)

# Name of the methods, columns with variable names and columns with importance
regressionMethods <- c("bartMachine", "blasso", "bridge", "cforest", "lasso","monmlp", "qrf", "rf", "svmPoly", "xgbLinear")
names  <- paste0("V", seq(1, 19, 2))
columns <- paste0("V", seq(2, 20, 2))


# Sort columns by name
variables <- sort(importances$V1)
data <- data.frame(Variable = variables)
for(idx in 1:10) {
  data[[regressionMethods[idx]]] <- importances[[columns[idx]]][order(importances[[names[idx]]])]
}

# Data preprocessing
standardize <- function(x) (x-min(x))/(max(x)-min(x))
for(method in regressionMethods) {
  data[[method]] <- standardize(data[[method]])
}
data <- t(data[, -c(1)])
colnames(data) <- variables

# Frideman test
friedmanTest(data)

# Ranking of variables
ranking <- rankMatrix(data)
ranking <- rbind(ranking, AvgRank = colMeans(ranking))
ranking <- t(ranking)
ranking <- ranking[order(ranking[,11]),]

# Computation of significant differences
table <- friedmanPost(data, control = NULL)







