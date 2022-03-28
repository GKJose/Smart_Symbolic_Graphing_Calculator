# Abstract Network Architecture Overview
The network between arbitrary amount of calculators (clients) and a singular administrator (host) is one of a server-client network architecture, where the client constantly updates the administrator with information after every certain amount of time, as dictated by the administrator. The administrator is able to control the capabilities and functions of the calculator for as long as the client consents. However, the moment the client revokes consent the administrator will be immediately notified in order to maintain a controlled monitoring environment. Client and Host will be able to communicate via a RESTFUL api.
___
The host are able to perform the following actions:
- Create a server that is discoverable by clients.
- Able to directly modify the following capabilities of connected calculators:
	- Enable/Disable functions
		- In groups (predefined groups, as determined by the Giac manual)
			- Groups are defined in on the side of the administrator. The client knows not of the existence of groups.
		- Individually (Custom configurations can be saved locally to create groups)
	- Enable/Disable graphing 
		- By functionality (parametric plotting, finding extrema, finding intersection points, etc.)
		- In its entirety (disable Graphing tab) 
- Record the calculation histories and logging in varying amounts of detail
	- Each time a blacklisted function is attempted to be called
	- Each time a calculation is entered in the Main screen
	- Each time an expression is entered in the Graphing screen
	- Individual button presses
	- Journalctl log (advanced kernel stats)
- View of the calculator screens
	- Live (rapid screenshots that are sent to the host)
		- Can be set to notify the user that they are being monitored or not.
	- Intermittent screenshots
		- Can be set to a value by the administrator.
- Remotely connect to a tty of a calculator in the background
- Enforce settings onto calculators
	- Set allowed calculation modes
		- Approximate
		- Exact
- Send a payload of custom code files to calculators
- Stop monitoring
	- As an entire group
	- On an individual basis
___
The clients are able to do the following:
- Connect to a wireless networks
- Connect to a host and consent to monitoring
	- Restrictions imposed will be shown to the user 
	- Level of access being granted to host will be shown to the user
	- Host will not be able to connect to the client without first consenting
	- The client is able to revoke consent at any time
	- Restricted functions will be blocked
		- Error will be outputted and notification will be sent to administrators if set to do so
		- Certain keywords that are similar to blacklisted functions will not be able to be used

