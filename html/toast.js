;
(function () {

    var Toast = function Toast(element) {
        this.element = element;
        this.init();
    }

    Toast.prototype.onHidden = function () {
        if (!this.element.classList.contains(this.Constants.Hidden))
            this.element.classList.add(this.Constants.Hidden);
    }
    Toast.prototype.onShow = function (title, message) {
        if (!this.title) {
            this.title = this.element.querySelector(this.Constants.Title);
        }
        if (title && this.title)
            this.title.innerText = title;
        if (!this.message) {
            this.message = this.element.querySelector(this.Constants.Message);
        }
        if (message && this.message) {
            this.message.innerText = message;
        }
        if (this.element.classList.contains(this.Constants.Hidden))
            this.element.classList.remove(this.Constants.Hidden);
        this.hide();

    }
    Toast.prototype.Constants = {
        Hidden: "toast--hidden",
        Close: ".toast__close",
        Title: ".toast__type",
        Message: ".toast__message",
        HiddenDelay: 5000
    }

    Toast.prototype.init = function () {
        if (!this.element) return;
        this.close = this.element.querySelector(this.Constants.Close);
        if (this.close) {
            this.close.addEventListener('click', () => {});
        }
        this.hide();

    }
    Toast.prototype.hide = function () {
        setTimeout(() => this.onHidden(), this.Constants.HiddenDelay);

    }
    window['Toast'] = new Toast(document.querySelector('.toast__layout'));
})()