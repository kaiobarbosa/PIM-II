- A estrutura base deve seguir os conceitos do [[Clean Code]] e suportar perfeitamente as necessidades de um projeto voltado a [[Programação Orientada a Objeto]]

- Utilizaremos uma estrutura que herda muito dos conceitos da divisão  [[Structure MVC]] (model, view e controler). Entretanto, para seguirmos os padroes do [[Clean Code]] iremos reduzir as responsabilidades de cada pilar.

- Estrutura:

	- **Back-end**
		- config
		- src 
			- domain 
				- entitys 
				- validation
			- infrastructures 
				- external_services
				- repository 
			- presentation 
				- routes 
				- middlewares 
			- use_cases 

- **Como funciona?**

	- Backend -> o backend é também dividido em duas partes, src e config. 

		- **config ->** Guarda as principais configurações da aplicação, seja a configuração da conexão do banco, inicialização de um framework, etc.

		- **src ->**  É o repositório no qual guarda as funções de cada parte do backend

			- **domain ->** Guarda as entidade e as validações para que seja possível criar as entidades

			- **infrastructure ->** Guarda dois principais módulos, repository e external_services.
				- **repository ->** módulo responsável, unicamenta e exclusivamente, por agir diretamente com o banco de dados. Ele é o unico que pode de fato realizar ações no banco.

				 - **external_services ->** realiza funções que não foram criadas pelo autor do projeto
					 - Ex.: utilização de uma biblioteca para criptografia de senhas (bcrypt)

			- **presentation ->** guardas as routes e o middleware. Ou seja, responsavel por resgatar a requisição do front, ser o porteiro que vai analisar qual foi a requisição e encaminhar para o use_case correto. 
				- Vale ressaltar que middleware é onde vai ser validade/criado/administrado o jwt ou token de validação do usuário no sistema

			- **use_cases ->** responsável por tratar os dados da requisição e encaminhar para o repository para que a ação seja de fato concluída 